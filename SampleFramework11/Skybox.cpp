//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "Skybox.h"

#include "Utility.h"
#include "ShaderCompilation.h"

namespace SampleFramework11
{

Skybox::Skybox()
{
}

Skybox::~Skybox()
{
}

void Skybox::Initialize(ID3D11Device* device)
{
    // Load the shaders
    ID3D10BlobPtr byteCode;
    vertexShader.Attach(CompileVSFromFile(device, L"SampleFramework11\\Shaders\\Skybox.hlsl", "SkyboxVS", "vs_4_0", NULL, NULL, &byteCode));
    pixelShader.Attach(CompilePSFromFile(device, L"SampleFramework11\\Shaders\\Skybox.hlsl", "SkyboxPS"));

    // Create the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DXCall(device->CreateInputLayout(layout, 1, byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &inputLayout));

    // Create and initialize the vertex and index buffers
    XMFLOAT3 verts[NumVertices] =
    {
        XMFLOAT3(-1, 1, 1),
        XMFLOAT3(1, 1, 1),
        XMFLOAT3(1, -1, 1),
        XMFLOAT3(-1, -1, 1),
        XMFLOAT3(1, 1, -1),
        XMFLOAT3(-1, 1, -1),
        XMFLOAT3(-1, -1, -1),
        XMFLOAT3(1, -1,- 1),
    };

    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.ByteWidth = sizeof(verts);
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = verts;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    DXCall(device->CreateBuffer(&desc, &initData, &vertexBuffer));

    unsigned short indices[NumIndices] =
    {
        0, 1, 2, 2, 3, 0,   // Front
        1, 4, 7, 7, 2, 1,   // Right
        4, 5, 6, 6, 7, 4,   // Back
        5, 0, 3, 3, 6, 5,   // Left
        5, 4, 1, 1, 0, 5,   // Top
        3, 2, 7, 7, 6, 3    // Bottom
    };

    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.ByteWidth = sizeof(indices);
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    initData.pSysMem = indices;
    DXCall(device->CreateBuffer(&desc, &initData, &indexBuffer));

    // Create the constant buffer
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = CBSize(sizeof(VSConstants));
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    DXCall(device->CreateBuffer(&desc, NULL, &constantBuffer));

    // Create a depth-stencil state
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;
    DXCall(device->CreateDepthStencilState(&dsDesc, &dsState));

    // Create a blend state
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    for (UINT i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = false;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }
    DXCall(device->CreateBlendState(&blendDesc, &blendState));

    // Create a rasterizer state
    D3D11_RASTERIZER_DESC rastDesc;
    rastDesc.AntialiasedLineEnable = FALSE;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = TRUE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = false;
    rastDesc.SlopeScaledDepthBias = 0;
    DXCall(device->CreateRasterizerState(&rastDesc, &rastState));

    D3D11_SAMPLER_DESC sampDesc;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.BorderColor[0] = 0;
    sampDesc.BorderColor[1] = 0;
    sampDesc.BorderColor[2] = 0;
    sampDesc.BorderColor[3] = 0;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.MinLOD = 0;
    sampDesc.MipLODBias = 0;
    DXCall(device->CreateSamplerState(&sampDesc, &samplerState));
}

void Skybox::Render(ID3D11DeviceContext* context,
                    ID3D11ShaderResourceView* environmentMap,
                    const XMMATRIX& view,
                    const XMMATRIX& projection,
                    XMFLOAT3 bias,
                    bool setStates)
{
    D3DPERF_BeginEvent(0xFFFFFFFF, L"Skybox Render");

    if(setStates)
    {
        float blendFactor[4] = {1, 1, 1, 1};
        context->RSSetState(rastState);
        context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
        context->OMSetDepthStencilState(dsState, 0);
        context->PSSetSamplers(0, 1, &(samplerState.GetInterfacePtr()));
    }

    // Get the viewports
    UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    context->RSGetViewports(&numViewports, viewports);

    // Set a viewport with MinZ pushed back
    D3D11_VIEWPORT vp = viewports[0];
    vp.MinDepth = 1.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    // Set the input layout
    context->IASetInputLayout(inputLayout);

    // Set the vertex buffer
    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    ID3D11Buffer* vertexBuffers[1] = { vertexBuffer.GetInterfacePtr() };
    context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);

    // Set the index buffer
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the shaders
    context->VSSetShader(vertexShader, NULL, 0);
    context->PSSetShader(pixelShader, NULL, 0);
    context->GSSetShader(NULL, NULL, 0);
    context->DSSetShader(NULL, NULL, 0);
    context->HSSetShader(NULL, NULL, 0);

    // Set the constants
    D3D11_MAPPED_SUBRESOURCE mapped;
    DXCall(context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    VSConstants* constants = reinterpret_cast<VSConstants*>(mapped.pData);
    constants->View = XMMatrixTranspose(view);
    constants->Projection = XMMatrixTranspose(projection);
    constants->Bias = bias;
    context->Unmap(constantBuffer, 0);

    ID3D11Buffer* buffers[1] = { constantBuffer };
    context->VSSetConstantBuffers(0, 1, buffers);

    // Set the texture
    context->PSSetShaderResources(0, 1, &environmentMap);

    // Draw
    context->DrawIndexed(NumIndices, 0, 0);

    // Set the viewport back to what it was
    context->RSSetViewports(numViewports, viewports);

    D3DPERF_EndEvent();
}

}