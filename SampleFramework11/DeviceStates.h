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

class BlendStates
{

protected:

    ID3D11BlendStatePtr blendDisabled;
    ID3D11BlendStatePtr additiveBlend;
    ID3D11BlendStatePtr alphaBlend;
    ID3D11BlendStatePtr pmAlphaBlend;
    ID3D11BlendStatePtr noColor;

public:

    void Initialize(ID3D11Device* device);

    ID3D11BlendState* BlendDisabled () { return blendDisabled; };
    ID3D11BlendState* AdditiveBlend () { return additiveBlend; };
    ID3D11BlendState* AlphaBlend () { return alphaBlend; };
    ID3D11BlendState* PreMultipliedAlphaBlend () { return pmAlphaBlend; };
    ID3D11BlendState* ColorWriteDisabled () { return noColor; };

    static D3D11_BLEND_DESC BlendDisabledDesc();
    static D3D11_BLEND_DESC AdditiveBlendDesc();
    static D3D11_BLEND_DESC AlphaBlendDesc();
    static D3D11_BLEND_DESC PreMultipliedAlphaBlendDesc();
    static D3D11_BLEND_DESC ColorWriteDisabledDesc();
};


class RasterizerStates
{

protected:

    ID3D11RasterizerStatePtr noCull;
    ID3D11RasterizerStatePtr cullBackFaces;
    ID3D11RasterizerStatePtr cullFrontFaces;
    ID3D11RasterizerStatePtr noCullNoMS;

public:

    void Initialize(ID3D11Device* device);

    ID3D11RasterizerState* NoCull() { return noCull; };
    ID3D11RasterizerState* BackFaceCull() { return cullBackFaces; };
    ID3D11RasterizerState* FrontFaceCull() { return cullFrontFaces; };
    ID3D11RasterizerState* NoCullNoMS() { return noCullNoMS; };

    static D3D11_RASTERIZER_DESC NoCullDesc();
    static D3D11_RASTERIZER_DESC FrontFaceCullDesc();
    static D3D11_RASTERIZER_DESC BackFaceCullDesc();
    static D3D11_RASTERIZER_DESC NoCullNoMSDesc();

};


class DepthStencilStates
{
    ID3D11DepthStencilStatePtr depthDisabled;
    ID3D11DepthStencilStatePtr depthEnabled;
    ID3D11DepthStencilStatePtr revDepthEnabled;
    ID3D11DepthStencilStatePtr depthWriteEnabled;
    ID3D11DepthStencilStatePtr revDepthWriteEnabled;

public:

    void Initialize(ID3D11Device* device);

    ID3D11DepthStencilState* DepthDisabled() { return depthDisabled; };
    ID3D11DepthStencilState* DepthEnabled() { return depthEnabled; };
    ID3D11DepthStencilState* ReverseDepthEnabled() { return revDepthEnabled; };
    ID3D11DepthStencilState* DepthWriteEnabled() { return depthWriteEnabled; };
    ID3D11DepthStencilState* ReverseDepthWriteEnabled() { return revDepthWriteEnabled; };

    static D3D11_DEPTH_STENCIL_DESC DepthDisabledDesc();
    static D3D11_DEPTH_STENCIL_DESC DepthEnabledDesc();
    static D3D11_DEPTH_STENCIL_DESC ReverseDepthEnabledDesc();
    static D3D11_DEPTH_STENCIL_DESC DepthWriteEnabledDesc();
    static D3D11_DEPTH_STENCIL_DESC ReverseDepthWriteEnabledDesc();
};


class SamplerStates
{

    ID3D11SamplerStatePtr linear;
    ID3D11SamplerStatePtr point;
    ID3D11SamplerStatePtr anisotropic;

public:

    void Initialize(ID3D11Device* device);

    ID3D11SamplerState* Linear() { return linear; };
    ID3D11SamplerState* Point() { return point; };
    ID3D11SamplerState* Anisotropic() { return anisotropic; };

    static D3D11_SAMPLER_DESC LinearDesc();
    static D3D11_SAMPLER_DESC PointDesc();
    static D3D11_SAMPLER_DESC AnisotropicDesc();
};

}
