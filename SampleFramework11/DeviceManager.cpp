//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "DeviceManager.h"
#include "Exceptions.h"
#include "Utility.h"

namespace SampleFramework11
{

DeviceManager::DeviceManager()	:	backBufferFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
									backBufferWidth(1280),
									backBufferHeight(720),
									msCount(1),
									msQuality(0),
									enableAutoDS(true),
									fullScreen(false),
									featureLevel(D3D_FEATURE_LEVEL_11_0),
									autoDSFormat(DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT),
                                    useAutoDSAsSR(false)
{
    refreshRate.Numerator = 60;
    refreshRate.Denominator = 1;
}

DeviceManager::~DeviceManager()
{
	if (immediateContext)
    {
		immediateContext->ClearState();
        immediateContext->Flush();
    }
}

void DeviceManager::Initialize(HWND outputWindow)
{
    CheckForSuitableOutput();

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

    if(fullScreen)
        PrepareFullScreenSettings();
    else
    {
        refreshRate.Numerator = 60;
        refreshRate.Denominator = 1;
    }

	desc.BufferCount = 2;
	desc.BufferDesc.Format = backBufferFormat;
	desc.BufferDesc.Width = backBufferWidth;
	desc.BufferDesc.Height = backBufferHeight;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.RefreshRate = refreshRate;
	desc.SampleDesc.Count = msCount;
	desc.SampleDesc.Quality = msQuality;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.OutputWindow = outputWindow;
	desc.Windowed = !fullScreen;

	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

	bool useDebugDevice = false;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	useDebugDevice = true;
#endif

	DXCall(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags,
		NULL, 0, D3D11_SDK_VERSION, &desc, &swapChain, &device, NULL, &immediateContext));

	if(useDebugDevice && D3DPERF_GetStatus() == 0)
	{
		ID3D11InfoQueue* infoQueue = NULL;
		DXCall(device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&infoQueue)));
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->Release();
	}

    AfterReset();
}

void DeviceManager::AfterReset()
{
    DXCall(swapChain->GetBuffer(0, __uuidof(bbTexture), reinterpret_cast<void**>(&bbTexture)));
    DXCall(device->CreateRenderTargetView(bbTexture, NULL, &bbRTView));

    // Create a default DepthStencil buffer
    if(enableAutoDS)
    {
        UINT bindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (useAutoDSAsSR)
            bindFlags |= D3D11_BIND_SHADER_RESOURCE;

        DXGI_FORMAT dsTexFormat;
        if (!useAutoDSAsSR)
            dsTexFormat = autoDSFormat;
        else if (autoDSFormat == DXGI_FORMAT_D16_UNORM)
            dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
        else if(autoDSFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
            dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
        else
            dsTexFormat = DXGI_FORMAT_R32_TYPELESS;

        D3D11_TEXTURE2D_DESC dsDesc;
        dsDesc.Width = backBufferWidth;
        dsDesc.Height = backBufferHeight;
        dsDesc.ArraySize = 1;
        dsDesc.BindFlags = bindFlags;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.Format = dsTexFormat;
        dsDesc.MipLevels = 1;
        dsDesc.MiscFlags = 0;
        dsDesc.SampleDesc.Count = msCount;
        dsDesc.SampleDesc.Quality = msQuality;
        dsDesc.Usage = D3D11_USAGE_DEFAULT;
        DXCall(device->CreateTexture2D(&dsDesc, NULL, &autoDSTexture));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = autoDSFormat;
        dsvDesc.ViewDimension = msCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = 0;
        DXCall(device->CreateDepthStencilView(autoDSTexture, &dsvDesc, &autoDSView));

        if (useAutoDSAsSR)
        {
            DXGI_FORMAT dsSRVFormat;
            if (autoDSFormat == DXGI_FORMAT_D16_UNORM)
                dsSRVFormat = DXGI_FORMAT_R16_UNORM;
            else if(autoDSFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
                dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS ;
            else
                dsSRVFormat = DXGI_FORMAT_R32_FLOAT;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = dsSRVFormat;
            srvDesc.ViewDimension = msCount > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            DXCall(device->CreateShaderResourceView(autoDSTexture, &srvDesc, &autoDSSRView));
        }
        else
            autoDSSRView = NULL;
    }

    // Set default render targets
    immediateContext->OMSetRenderTargets(1, &(bbRTView.GetInterfacePtr()), autoDSView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(backBufferWidth);
    vp.Height = static_cast<float>(backBufferHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    immediateContext->RSSetViewports(1, &vp);
}

void DeviceManager::CheckForSuitableOutput()
{
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));
    if (FAILED(hr))
        throw Exception(L"Unable to create a DXGI 1.1 device.\n Make sure your OS and driver support DirectX 11");

    // Look for an adapter that supports D3D11
    IDXGIAdapter1Ptr curAdapter;
    UINT adapterIdx = 0;
    while(!adapter && SUCCEEDED(factory->EnumAdapters1(0, &adapter)))
        if(SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(ID3D11Device), NULL)))
            adapter = curAdapter;

    if (!adapter)
        throw Exception(L"Unable to locate a DXGI 1.1 adapter that supports a D3D11 device.\n Make sure your OS and driver support DirectX 11");

    // We'll just use the first output
    DXCall(adapter->EnumOutputs(0, &output));
}

void DeviceManager::PrepareFullScreenSettings()
{
    _ASSERT(output);

    // Have the Output look for the closest matching mode
    DXGI_MODE_DESC desiredMode;
    desiredMode.Format = backBufferFormat;
    desiredMode.Width = backBufferWidth;
    desiredMode.Height = backBufferHeight;
    desiredMode.RefreshRate.Numerator = 0;
    desiredMode.RefreshRate.Denominator = 0;
    desiredMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    desiredMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    DXGI_MODE_DESC closestMatch;
    DXCall(output->FindClosestMatchingMode(&desiredMode, &closestMatch, device.GetInterfacePtr()));

    backBufferFormat = closestMatch.Format;
    backBufferWidth = closestMatch.Width;
    backBufferHeight = closestMatch.Height;
    refreshRate = closestMatch.RefreshRate;
}

void DeviceManager::Reset()
{
    _ASSERT(swapChain);

    // Release all references
    if (bbTexture)
        bbTexture.Release();

    if (bbRTView)
        bbRTView.Release();

    if (autoDSTexture)
        autoDSTexture.Release();

    if (autoDSView)
        autoDSView.Release();

    if (autoDSSRView)
        autoDSSRView.Release();

    immediateContext->ClearState();

    if(fullScreen)
        PrepareFullScreenSettings();
    else
    {
        refreshRate.Numerator = 60;
        refreshRate.Denominator = 1;
    }

    DXCall(swapChain->SetFullscreenState(fullScreen, NULL));

    DXCall(swapChain->ResizeBuffers(2, backBufferWidth, backBufferHeight, backBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    if(fullScreen)
    {
        DXGI_MODE_DESC mode;
        mode.Format = backBufferFormat;
        mode.Width = backBufferWidth;
        mode.Height = backBufferHeight;
        mode.RefreshRate.Numerator = 0;
        mode.RefreshRate.Denominator = 0;
        mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        DXCall(swapChain->ResizeTarget(&mode));
    }

    AfterReset();
}

void DeviceManager::Present()
{
    _ASSERT(device);

	UINT interval = vsync ? 1 : 0;
	DXCall(swapChain->Present(interval, 0));
}

}