//======================================================================
//
//	MSAA Sample Pattern Inspector
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "SamplePattern.h"

#include "resource.h"
#include "SampleFramework11/InterfacePointers.h"
#include "SampleFramework11/Window.h"
#include "SampleFramework11/DeviceManager.h"
#include "SampleFramework11/Input.h"
#include "SampleFramework11/SpriteRenderer.h"
#include "SampleFramework11/Model.h"
#include "SampleFramework11/Utility.h"
#include "SampleFramework11/Camera.h"
#include "SampleFramework11/ShaderCompilation.h"

#define UseNVAPI_ (1)

#if UseNVAPI_

#include "NVAPI/nvapi.h"
#pragma comment(lib, "NVAPI/amd64/nvapi64.lib")

#endif

using namespace SampleFramework11;
using std::wstring;

const UINT WindowWidth = 800;
const UINT WindowHeight = 800;
const float WindowWidthF = static_cast<float>(WindowWidth);
const float WindowHeightF = static_cast<float>(WindowHeight);

// Number of "buckets" for subsample X and Y coordinates in D3D (4-bit precision)
static const UINT SampleRes = 16;

// Computes a radical inverse with base 2 using crazy bit-twiddling from "Hacker's Delight"
static float RadicalInverseBase2(UINT bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

// Returns a single 2D point in a Hammersley sequence of length "numSamples", using base 1 and base 2
static XMFLOAT2 Hammersley2D(UINT sampleIdx, UINT numSamples)
{
	return XMFLOAT2(float(sampleIdx) / float(numSamples), RadicalInverseBase2(sampleIdx));
}


SamplePattern::SamplePattern() :  App(L"Sample Pattern Inspector", MAKEINTRESOURCEW(IDI_DEFAULT))
{
	deviceManager.SetBackBufferWidth(WindowWidth);
	deviceManager.SetBackBufferHeight(WindowHeight);

	window.SetClientArea(WindowWidth, WindowHeight);

    currMSAAMode = 0;
	useCustomSampling = false;
	nvExtensionsAvailable = false;
}

void SamplePattern::BeforeReset()
{

}

void SamplePattern::AfterReset()
{
}

void SamplePattern::LoadContent()
{
    ID3D11DevicePtr device = deviceManager.Device();
    ID3D11DeviceContextPtr deviceContext = deviceManager.ImmediateContext();

    // Create a font + SpriteRenderer
    font.Initialize(L"Consolas", 18, SpriteFont::Regular, true, device);
    smallFont.Initialize(L"Microsoft Sans Serif", 8.5f, SpriteFont::Regular, true, device);
    spriteRenderer.Initialize(device);

    // Load the 1x1 white texture
    DXCall(D3DX11CreateShaderResourceViewFromFile(device, L"Content\\White.png", NULL, NULL, &whiteTexture, NULL));

    // Enumerate MSAA modes
    for (UINT numSamples = 1; numSamples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++numSamples)
    {
        UINT numQualityLevels = 0;
        DXCall(device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, numSamples, &numQualityLevels));
        for (UINT quality = 0; quality < numQualityLevels; ++quality)
        {
            DXGI_SAMPLE_DESC desc;
            desc.Count = numSamples;
            desc.Quality = quality;
            msaaModes.push_back(desc);
        }

        // Add the standard MSAA patterns, if they're supported by the device
        if (deviceManager.FeatureLevel() >= D3D_FEATURE_LEVEL_10_1 && numQualityLevels != 0)
        {
            DXGI_SAMPLE_DESC desc;
            desc.Count = numSamples;
            desc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
            msaaModes.push_back(desc);

            desc.Quality = D3D11_CENTER_MULTISAMPLE_PATTERN;
            msaaModes.push_back(desc);
        }
    }

	patternDetectConstants.Initialize(device);

    SetupMSAAMode();

#if UseNVAPI_
	if(NvAPI_Initialize() != NVAPI_OK)
		return;

	NvAPI_D3D11_RASTERIZER_DESC_EX rsDesc;
	ZeroMemory(&rsDesc, sizeof(rsDesc));

	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 1.0f;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthClipEnable = FALSE;
	rsDesc.ScissorEnable = FALSE;
	rsDesc.MultisampleEnable = TRUE;
	rsDesc.AntialiasedLineEnable = FALSE;
	rsDesc.ForcedSampleCount = 0;
	rsDesc.ConservativeRasterEnable = false;
	rsDesc.QuadFillMode = NVAPI_QUAD_FILLMODE_DISABLED;
	rsDesc.PostZCoverageEnable = false;
	rsDesc.CoverageToColorEnable = false;
	rsDesc.CoverageToColorRTIndex = 0;
	rsDesc.ProgrammableSamplePositionsEnable = true;
	rsDesc.InterleavedSamplingEnable = true;

	rsDesc.SampleCount = 2;
	UINT quadrantCountsX[4] = { 0 };
	UINT quadrantCountsY[4] = { 0 };
	for(UINT i = 0; i < 8; ++i)
	{
		XMFLOAT2 samplePos = Hammersley2D(i, 8);
		samplePos.x *= 2.0f;
		samplePos.y *= 2.0f;
		UINT quadrantX = samplePos.x >= 1.0f ? 1 : 0;
		UINT quadrantY = samplePos.y >= 1.0f ? 1 : 0;
		UINT quadrant = quadrantY * 2 + quadrantX;
		UINT sampleIdxX = quadrantCountsX[quadrant]++;
		UINT sampleIdxY = quadrantCountsY[quadrant]++;
		UINT idxOffset = quadrant * 2;

		if(quadrantX)
			samplePos.x -= 1.0f;
		if(quadrantY)
			samplePos.y -= 1.0f;

		rsDesc.SamplePositionsX[sampleIdxX + idxOffset] = UINT8(Clamp(samplePos.x * 16.0f, 0.0f, 15.0f));
		rsDesc.SamplePositionsY[sampleIdxY + idxOffset] = UINT8(Clamp(samplePos.y * 16.0f, 0.0f, 15.0f));
	}

	NvAPI_Status status = NvAPI_D3D11_CreateRasterizerState(device, &rsDesc, &msaa2xRState);
	if(status != NVAPI_OK)
		return;

	rsDesc.SampleCount = 4;
	ZeroMemory(quadrantCountsX, sizeof(quadrantCountsX));
	ZeroMemory(quadrantCountsY, sizeof(quadrantCountsY));
	for(UINT i = 0; i < 16; ++i)
	{
		XMFLOAT2 samplePos = Hammersley2D(i, 16);
		samplePos.x *= 2.0f;
		samplePos.y *= 2.0f;
		UINT quadrantX = samplePos.x >= 1.0f ? 1 : 0;
		UINT quadrantY = samplePos.y >= 1.0f ? 1 : 0;
		UINT quadrant = quadrantY * 2 + quadrantX;
		UINT sampleIdxX = quadrantCountsX[quadrant]++;
		UINT sampleIdxY = quadrantCountsY[quadrant]++;
		UINT idxOffset = quadrant * 4;

		if(quadrantX)
			samplePos.x -= 1.0f;
		if(quadrantY)
			samplePos.y -= 1.0f;

        rsDesc.SamplePositionsX[sampleIdxX + idxOffset] = UINT8(Clamp(samplePos.x * 16.0f, 0.0f, 15.0f));
        rsDesc.SamplePositionsY[sampleIdxY + idxOffset] = UINT8(Clamp(samplePos.y * 16.0f, 0.0f, 15.0f));
	}

	status = NvAPI_D3D11_CreateRasterizerState(device, &rsDesc, &msaa4xRState);
	if(status != NVAPI_OK)
		return;

	rsDesc.SampleCount = 8;
	ZeroMemory(quadrantCountsX, sizeof(quadrantCountsX));
	ZeroMemory(quadrantCountsY, sizeof(quadrantCountsY));
	for(UINT i = 0; i < 16; ++i)
	{
		XMFLOAT2 samplePos = Hammersley2D(i, 16);

		// For 8x modes, the 16 sample positions describe the left two pixels in the quad
		samplePos.x *= 1.0f;
		samplePos.y *= 2.0f;
		UINT quadrantX = 0;
		UINT quadrantY = samplePos.y >= 1.0f ? 1 : 0;
		UINT quadrant = quadrantY;
		UINT sampleIdxX = quadrantCountsX[quadrant]++;
		UINT sampleIdxY = quadrantCountsY[quadrant]++;
		UINT idxOffset = quadrant * 8;

		if(quadrantY)
			samplePos.y -= 1.0f;

        rsDesc.SamplePositionsX[sampleIdxX + idxOffset] = UINT8(Clamp(samplePos.x * 16.0f, 0.0f, 15.0f));
        rsDesc.SamplePositionsY[sampleIdxY + idxOffset] = UINT8(Clamp(samplePos.y * 16.0f, 0.0f, 15.0f));
	}

	status = NvAPI_D3D11_CreateRasterizerState(device, &rsDesc, &msaa8xRState);
	if(status != NVAPI_OK)
		return;

	nvExtensionsAvailable = true;
#endif // UseNVAPI_
}

// Sets up ever
void SamplePattern::SetupMSAAMode()
{
    ID3D11Device* device = deviceManager.Device();
    DXGI_SAMPLE_DESC desc = msaaModes[currMSAAMode];

    // For defining macro values
    static const LPCSTR Indices[D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT] =
    {   "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "10", "11", "12", "13", "14", "15",
        "16","17", "18", "19", "20", "21", "22", "23",
        "24", "25", "26", "27", "28", "29", "30", "31",
    };

    // Load our shaders
    D3D10_SHADER_MACRO macros[4];
    macros[0].Name = "NumSamples";
    macros[0].Definition = Indices[desc.Count];
    macros[1].Name = "SampleIndex";
	macros[2].Name = "MSAAEnabled";
	macros[2].Definition = desc.Count > 1 ? "1" : "0";
    macros[3].Name = 0;
    macros[3].Definition = 0;
    for(UINT i = 0; i < desc.Count; ++i)
    {
        macros[1].Definition = Indices[i];
        patternDetectShaders[i].Attach(CompilePSFromFile(device, L"PatternDetect.hlsl", "PS", "ps_4_0", macros));
    }

    // Create our render targets
    sampleTarget.Initialize(device, 2, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 1, desc.Count, desc.Quality);
    patternTarget.Initialize(device, desc.Count, 4, DXGI_FORMAT_R32G32_FLOAT);

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.ArraySize = 1;
    texDesc.BindFlags = 0;
    texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
    texDesc.Width = desc.Count;
    texDesc.Height = 4;
    texDesc.MipLevels = 1;
    texDesc.MiscFlags = 0;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    DXCall(device->CreateTexture2D(&texDesc, NULL, &stagingTexture));
}

void SamplePattern::Update(const Timer& timer)
{
    MouseState mouseState = MouseState::GetMouseState(window);
    KeyboardState kbState = KeyboardState::GetKeyboardState();

    if (kbState.IsKeyDown(Keys::Escape))
        window.Destroy();

    // Switch MSAA mode
    if (kbState.RisingEdge(Keys::Up) || kbState.RisingEdge(Keys::M))
    {
        currMSAAMode = (currMSAAMode + 1) % msaaModes.size();
        SetupMSAAMode();
    }

    if (kbState.RisingEdge(Keys::Down) || kbState.RisingEdge(Keys::N))
    {
        if(currMSAAMode == 0)
            currMSAAMode = static_cast<UINT>(msaaModes.size() - 1);
        else
            --currMSAAMode;
        SetupMSAAMode();
    }

	if(nvExtensionsAvailable && kbState.RisingEdge(Keys::K))
		useCustomSampling = !useCustomSampling;
}

void SamplePattern::Render(const Timer& timer)
{
    ID3D11DeviceContextPtr context = deviceManager.ImmediateContext();

    ID3D11RenderTargetView* renderTargets[1] = { sampleTarget.RTView };
    context->OMSetRenderTargets(1, renderTargets, NULL);
    float clearColor[4] = {0, 0, 0, 1};
    context->ClearRenderTargetView(sampleTarget.RTView, clearColor);

    D3D11_VIEWPORT vp;
    vp.Width = 2.0f;
    vp.Height = 2.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    context->RSSetViewports(1, &vp);

    // Render samples
    spriteRenderer.Begin(context, SpriteRenderer::Point);

	UINT numSamples = msaaModes[currMSAAMode].Count;
	ID3D11RasterizerState* rsState = NULL;
	if(nvExtensionsAvailable && useCustomSampling)
	{
		if(numSamples == 2)
			rsState = msaa2xRState;
		else if(numSamples == 4)
			rsState = msaa4xRState;
		else if(numSamples == 8)
			rsState = msaa8xRState;
	}

	if(rsState != NULL)
		context->RSSetState(rsState);

    XMMATRIX scale = XMMatrixScaling(1.0f / SampleRes, 1.0f / SampleRes, 1.0f);
	for(UINT quadPixelIdx = 0; quadPixelIdx < 4; ++quadPixelIdx)
	{
		float quadOffsetX = float(quadPixelIdx % 2);
		float quadOffsetY = float(quadPixelIdx / 2);

		for (UINT y = 0; y < SampleRes; y++)
		{
			for (UINT x = 0; x < SampleRes; x++)
			{
				XMFLOAT4 color;
				color.x = float(x) / SampleRes;
				color.y = float(y) / SampleRes;
				color.z = 1.0f;
				color.w = 1.0f;
				XMMATRIX transform = scale * XMMatrixTranslation((x - 0.5f) / SampleRes + quadOffsetX, (y - 0.5f) / SampleRes + quadOffsetY, 0);
				spriteRenderer.Render(whiteTexture, transform, color);
			}
		}
	}

    spriteRenderer.End();

    renderTargets[0] = patternTarget.RTView;
    context->OMSetRenderTargets(1, renderTargets, NULL);
    context->ClearRenderTargetView(patternTarget.RTView, clearColor);

    DXGI_SAMPLE_DESC desc = msaaModes[currMSAAMode];
    vp.Width = static_cast<float>(desc.Count);
    vp.Height = static_cast<float>(4.0f);
    context->RSSetViewports(1, &vp);

    // Pattern detect
    spriteRenderer.Begin(context);

    for(UINT i = 0; i < desc.Count; ++i)
    {
		context->PSSetShader(patternDetectShaders[i], NULL, 0);
		for(UINT quadPixelIdx = 0; quadPixelIdx < 4; ++quadPixelIdx)
		{
			patternDetectConstants.Data.PixelPosX = quadPixelIdx % 2;
			patternDetectConstants.Data.PixelPosY = quadPixelIdx / 2;
			patternDetectConstants.ApplyChanges(context);
			patternDetectConstants.SetPS(context, 2);

			XMMATRIX transform = XMMatrixTranslation(i * 1.0f, float(quadPixelIdx), 0);
			spriteRenderer.Render(sampleTarget.SRView, transform);
		}
    }

    spriteRenderer.End();

    renderTargets[0] = deviceManager.BackBuffer();
    context->OMSetRenderTargets(1, renderTargets, NULL);

    vp.Width = static_cast<float>(deviceManager.BackBufferWidth());
    vp.Height = static_cast<float>(deviceManager.BackBufferHeight());
    context->RSSetViewports(1, &vp);
    float backColor[4] = {0.292f, 0.484f, 0.929f, 1};
    context->ClearRenderTargetView(deviceManager.BackBuffer(), backColor);

    // Copy to the staging texture
    context->CopyResource(stagingTexture, patternTarget.Texture);

    RenderHUD();
}

void SamplePattern::RenderHUD()
{
    PIXEvent event(L"HUD Pass");

    DXGI_SAMPLE_DESC desc = msaaModes[currMSAAMode];

    // Get the pattern texture content
    D3D11_MAPPED_SUBRESOURCE mapped;
    DXCall(deviceManager.ImmediateContext()->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped));
    XMFLOAT2* pattern = reinterpret_cast<XMFLOAT2*>(mapped.pData);

	XMFLOAT2* quadPatterns[4] = { NULL };
	for(UINT i = 0; i < 4; ++i)
		quadPatterns[i] = reinterpret_cast<XMFLOAT2*>(reinterpret_cast<UINT8*>(mapped.pData) + mapped.RowPitch * i);

    spriteRenderer.Begin(deviceManager.ImmediateContext(), SpriteRenderer::Point);

    XMMATRIX transform = XMMatrixTranslation(25.0f, deviceManager.BackBufferHeight() * 0.65f, 0);

    wstring mode = L"Current MSAA Mode: ";
    wstring quality;
    if (desc.Quality == D3D11_STANDARD_MULTISAMPLE_PATTERN)
        quality = L"Standard MSAA Pattern";
    else if (desc.Quality == D3D11_CENTER_MULTISAMPLE_PATTERN)
        quality = L"Standard Center MSAA Pattern";
    else
        quality = L"Q" + ToString(desc.Quality);
    mode += ToString(desc.Count) + L"x, " + quality + L" (Press Up/Down or M/N to switch)";
    spriteRenderer.RenderText(font, mode.c_str(), transform);
    transform._42 += 20.0f;

	if(nvExtensionsAvailable)
	{
		wstring customPoints = L"Use Custom Sample Points: ";
		customPoints += useCustomSampling ? L"Yes" : L"No";
		customPoints += L" (Press K to switch)";
		spriteRenderer.RenderText(font, customPoints.c_str(), transform);
		transform._42 += 20.0f;
	}

	const wstring samplePosStrings[16] =
	{
		L"-0.5 (-8 / 16)",
		L"-0.4375 (-7 / 16)",
		L"-0.375 (-6 / 16)",
		L"-0.3125 (-5 / 16)",
		L"-0.25 (-4 / 16)",
		L"-0.1875 (-3 / 16)",
		L"-0.125 (-2 / 16)",
		L"-0.0625 (-1 / 16)",
		L"0.0 (0 / 16)",
		L"0.0625 (1 / 16)",
		L"0.125 (2 / 16)",
		L"0.1875 (3 / 16)",
		L"0.25 (4 / 16)",
		L"0.3125 (5 / 16)",
		L"0.375 (6 / 16)",
		L"0.4375 (7 / 16)",
	};

    for (UINT i = 0; i < desc.Count; ++i)
    {
		UINT samplePosX = min(UINT(pattern[i].x * SampleRes + 0.5f), 15);
		UINT samplePosY = min(UINT(pattern[i].y * SampleRes + 0.5f), 15);

        wstring text = L"Sample " + ToString(i) + L" - ";
        text += L"X: " + samplePosStrings[samplePosX];
		text += L" Y: " + samplePosStrings[samplePosY];

        spriteRenderer.RenderText(font, text.c_str(), transform);

        transform._42 += 20.0f;
    }

	for(UINT quadPixelIdx = 0; quadPixelIdx < 4; ++quadPixelIdx)
	{
		UINT quadOffsetX = quadPixelIdx % 2;
		UINT quadOffsetY = quadPixelIdx / 2;

		// Draw a great big pixel
		float pixelSize = deviceManager.BackBufferHeight() * 0.6f * 0.5f;
		float pixelDrawY = deviceManager.BackBufferHeight() * 0.035f;
		float pixelDrawX = (deviceManager.BackBufferWidth() / 2.0f) - (pixelSize);
		pixelDrawX += pixelSize * quadOffsetX;
		pixelDrawY += pixelSize * quadOffsetY;
		transform = XMMatrixScaling(pixelSize, pixelSize, 1.0f) * XMMatrixTranslation(pixelDrawX, pixelDrawY, 0);
		spriteRenderer.Render(whiteTexture, transform, XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));

		// Draw pixel borders
		float borderWidth = pixelSize * 0.02f;
		float borderHeight = pixelSize;
		float borderCenterX = pixelDrawX;
		float borderTopLeftX = borderCenterX - borderWidth * 0.5f;
		float borderTopLeftY = pixelDrawY;
		XMMATRIX borderTransform = XMMatrixScaling(borderWidth, borderHeight, 1.0f) * XMMatrixTranslation(borderTopLeftX, borderTopLeftY, 0);
		spriteRenderer.Render(whiteTexture, borderTransform, XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

		borderCenterX += pixelSize;
		borderTopLeftX = borderCenterX - borderWidth * 0.5f;
		borderTransform = XMMatrixScaling(borderWidth, borderHeight, 1.0f) * XMMatrixTranslation(borderTopLeftX, borderTopLeftY, 0);
		spriteRenderer.Render(whiteTexture, borderTransform, XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

		std::swap(borderWidth, borderHeight);
		borderTopLeftX = pixelDrawX;
		float borderCenterY = pixelDrawY;
		borderTopLeftY = borderCenterY - borderHeight * 0.5f;
		borderTransform = XMMatrixScaling(borderWidth, borderHeight, 1.0f) * XMMatrixTranslation(borderTopLeftX, borderTopLeftY, 0);
		spriteRenderer.Render(whiteTexture, borderTransform, XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

		borderCenterY += pixelSize;
		borderTopLeftY = borderCenterY - borderHeight * 0.5f;
		borderTransform = XMMatrixScaling(borderWidth, borderHeight, 1.0f) * XMMatrixTranslation(borderTopLeftX, borderTopLeftY, 0);
		spriteRenderer.Render(whiteTexture, borderTransform, XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

		float samplesize = pixelSize / SampleRes;
		float halfSampleSize = samplesize * 0.5f;

		// Draw the pixel center
		if (desc.Count > 1)
		{
			float centerPosX = pixelDrawX + (pixelSize * 0.5f) - halfSampleSize;
			float centerPosY = pixelDrawY + (pixelSize * 0.5f) - halfSampleSize;
			transform = XMMatrixScaling(samplesize, samplesize, 1.0f) * XMMatrixTranslation(centerPosX, centerPosY, 0);
			spriteRenderer.Render(whiteTexture, transform, XMFLOAT4(0.9f, 0.9f, 0.2f, 0.5f));
			transform = XMMatrixTranslation(centerPosX + halfSampleSize, centerPosY + halfSampleSize, 0);
		}

		// Draw the sample points
		for (UINT sample = 0; sample < desc.Count; ++sample)
		{
			XMFLOAT2 samplePos = quadPatterns[quadPixelIdx][sample];
			samplePos.x = std::floor(samplePos.x * SampleRes + 0.5f) / SampleRes;
			samplePos.y = std::floor(samplePos.y * SampleRes + 0.5f) / SampleRes;
			float samplePosX = pixelDrawX + (pixelSize * samplePos.x) - halfSampleSize;
			float samplePosY = pixelDrawY + (pixelSize * samplePos.y) - halfSampleSize;
			transform = XMMatrixScaling(samplesize, samplesize, 1.0f) * XMMatrixTranslation(samplePosX, samplePosY, 0);
			spriteRenderer.Render(whiteTexture, transform, XMFLOAT4(0.9f, 0.2f, 0.2f, 0.5f));
			transform = XMMatrixTranslation(samplePosX + halfSampleSize * 0.5f, samplePosY + halfSampleSize * 0.5f, 0);
			spriteRenderer.RenderText(smallFont, ToString(sample).c_str(), transform, XMFLOAT4(1, 1, 1, 1));
		}
	}

    spriteRenderer.End();

    deviceManager.ImmediateContext()->Unmap(stagingTexture, 0);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	SamplePattern app;
    app.Run();
}


