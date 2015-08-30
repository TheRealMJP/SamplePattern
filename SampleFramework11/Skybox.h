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

class Skybox
{

public:

    Skybox();
    ~Skybox();

    void Initialize(ID3D11Device* device);

    void Render(ID3D11DeviceContext* context,
                ID3D11ShaderResourceView* environmentMap,
                const XMMATRIX& view,
                const XMMATRIX& projection,
                XMFLOAT3 bias = XMFLOAT3(1, 1, 1),
                bool setStates = true);

protected:

    static const UINT_PTR NumIndices = 36;
    static const UINT_PTR NumVertices = 8;

    struct VSConstants
    {
        XMMATRIX View;
        XMMATRIX Projection;
        XMFLOAT3 Bias;
    };

    ID3D11VertexShaderPtr vertexShader;
    ID3D11PixelShaderPtr pixelShader;
    ID3D11InputLayoutPtr inputLayout;
    ID3D11BufferPtr vertexBuffer;
    ID3D11BufferPtr indexBuffer;
    ID3D11BufferPtr constantBuffer;
    ID3D11DepthStencilStatePtr dsState;
    ID3D11BlendStatePtr blendState;
    ID3D11RasterizerStatePtr rastState;
    ID3D11SamplerStatePtr samplerState;
};

}