//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#pragma once

#include "PCH.h"

#include "InterfacePointers.h"

namespace SampleFramework11
{

class PostProcessor
{

public:

    PostProcessor();
    ~PostProcessor();

    virtual void Initialize(ID3D11Device* device);

    virtual void Render(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);

    virtual void AfterReset(UINT width, UINT height);

protected:

    static const UINT_PTR MaxInputs = 4;

    struct TempRenderTarget
    {
        ID3D11Texture2D* Texture;
        ID3D11ShaderResourceView* SRView;
        ID3D11RenderTargetView* RTView;
        UINT Width;
        UINT Height;
        DXGI_FORMAT Format;
        UINT MSCount;
        UINT MSQuality;
        BOOL InUse;
    };

    struct PSConstants
    {
        XMFLOAT2 InputSize[MaxInputs];
        XMFLOAT2 OutputSize;
    };

    struct QuadVertex
    {
        XMFLOAT4 Position;
        XMFLOAT2 TexCoord;
    };

    TempRenderTarget* GetTempRenderTarget(UINT width, UINT height, DXGI_FORMAT format, UINT msCount = 1, UINT msQuality = 0);
    void ClearTempRenderTargetCache();

    void PostProcess(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output, ID3D11PixelShader* pixelShader, const WCHAR* name);
    virtual void PostProcess(ID3D11PixelShader* pixelShader, const WCHAR* name);

    std::vector<TempRenderTarget*> tempRenderTargets;
    ID3D11Device* device;
    ID3D11DeviceContext* context;

    // Full screen quad
    ID3D11BufferPtr quadVB;
    ID3D11BufferPtr quadIB;
    ID3D11VertexShaderPtr quadVS;
    ID3D11InputLayoutPtr quadInputLayout;

    // Sampler states
    ID3D11SamplerStatePtr pointSamplerState;
    ID3D11SamplerStatePtr linearSamplerState;

    // Device states
    ID3D11DepthStencilStatePtr dsState;
    ID3D11BlendStatePtr blendState;
    ID3D11RasterizerStatePtr rastState;
    ID3D11SamplerStatePtr samplerState;

    // Inputs and outputs
    std::vector<ID3D11ShaderResourceView*> inputs;
    std::vector<ID3D11RenderTargetView*> outputs;

    // Constant buffer
    ID3D11BufferPtr psConstants;

    UINT inputWidth;
    UINT inputHeight;
};

}
