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

#include "Exceptions.h"
#include "Utility.h"
#include "InterfacePointers.h"

namespace SampleFramework11
{

class SpriteFont;

class SpriteRenderer
{

public:

    enum FilterMode
    {
        DontSet = 0,
        Linear = 1,
        Point = 2
    };

    static const UINT64 MaxBatchSize = 1000;

    struct SpriteDrawData
    {
        XMMATRIX Transform;
        XMFLOAT4 Color;
        XMFLOAT4 DrawRect;
    };

	SpriteRenderer();
	~SpriteRenderer();

	void Initialize(ID3D11Device* device);

    void Begin(ID3D11DeviceContext* deviceContext, FilterMode filterMode = DontSet);

	void Render(ID3D11ShaderResourceView* texture,
				const XMMATRIX& transform,
                const XMFLOAT4& color = XMFLOAT4(1, 1, 1, 1),
                const XMFLOAT4* drawRect = NULL);

    void RenderBatch(ID3D11ShaderResourceView* texture,
                     const SpriteDrawData* drawData,
                     UINT64 numSprites);

    void RenderText(const SpriteFont& font,
                    const WCHAR* text,
				    const XMMATRIX& transform,
                    const XMFLOAT4& color = XMFLOAT4(1, 1, 1, 1));

    void End();

protected:

    D3D11_TEXTURE2D_DESC SetPerBatchData(ID3D11ShaderResourceView* texture);

	ID3D11DevicePtr device;
	ID3D11VertexShaderPtr vertexShader;
    ID3D11VertexShaderPtr vertexShaderInstanced;
	ID3D11PixelShaderPtr pixelShader;
	ID3D11BufferPtr vertexBuffer;
	ID3D11BufferPtr indexBuffer;
    ID3D11BufferPtr vsPerBatchCB;
    ID3D11BufferPtr vsPerInstanceCB;
    ID3D11BufferPtr instanceDataBuffer;
	ID3D11InputLayoutPtr inputLayout;
    ID3D11InputLayoutPtr inputLayoutInstanced;
    ID3D11DeviceContextPtr context;

    ID3D11RasterizerStatePtr rastState;
    ID3D11DepthStencilStatePtr dsState;
    ID3D11BlendStatePtr alphaBlendState;
    ID3D11SamplerStatePtr linearSamplerState;
    ID3D11SamplerStatePtr pointSamplerState;

    bool initialized;

    SpriteDrawData textDrawData [MaxBatchSize];

    struct SpriteVertex
    {
        XMFLOAT2 Position;
        XMFLOAT2 TexCoord;
    };

    struct VSPerBatchCB
    {
        XMFLOAT2 TextureSize;
        XMFLOAT2 ViewportSize;
    };

};

}