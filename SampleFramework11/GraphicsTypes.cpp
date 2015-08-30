//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "GraphicsTypes.h"
#include "Exceptions.h"
#include "Utility.h"

namespace SampleFramework11
{

RenderTarget2D::RenderTarget2D() :  Width(0),
                                    Height(0),
                                    Format(DXGI_FORMAT_UNKNOWN),
                                    NumMipLevels(0),
                                    MultiSamples(0),
                                    MSQuality(0),
                                    AutoGenMipMaps(false)
{

}

void RenderTarget2D::Initialize(ID3D11Device* device,
							    UINT width,
							    UINT height,
							    DXGI_FORMAT format,
							    UINT numMipLevels,
							    UINT multiSamples,
							    UINT msQuality,
							    bool autoGenMipMaps)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.MipLevels = numMipLevels;
	desc.MiscFlags = (autoGenMipMaps && numMipLevels > 1) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	desc.SampleDesc.Count = multiSamples;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;
	DXCall(device->CreateTexture2D(&desc, NULL, &Texture));

    MipLevels.clear();
    for (UINT i = 0; i < numMipLevels; ++i)
    {
        ID3D11RenderTargetViewPtr rtView;
        D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
        rtDesc.Format = format;
        rtDesc.ViewDimension = multiSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
        rtDesc.Texture2D.MipSlice = i;
	    DXCall(device->CreateRenderTargetView(Texture, &rtDesc, &rtView));

        MipLevels.push_back(rtView);
    }

    RTView = MipLevels[0];

    DXCall(device->CreateShaderResourceView(Texture, NULL, &SRView));

	Width = width;
	Height = height;
	NumMipLevels = numMipLevels;
	MultiSamples = multiSamples;
	Format = format;
	AutoGenMipMaps = autoGenMipMaps;
};

DepthStencilBuffer::DepthStencilBuffer() :  Width(0),
                                            Height(0),
                                            MultiSamples(0),
                                            MSQuality(0),
                                            Format(DXGI_FORMAT_UNKNOWN)
{

}

void DepthStencilBuffer::Initialize(ID3D11Device* device,
									UINT width,
									UINT height,
									DXGI_FORMAT format,
									bool useAsShaderResource,
									UINT multiSamples,
									UINT msQuality)
{
	UINT bindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (useAsShaderResource)
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;

    DXGI_FORMAT dsTexFormat;
    if (!useAsShaderResource)
        dsTexFormat = format;
    else if (format == DXGI_FORMAT_D16_UNORM)
        dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
    else if(format == DXGI_FORMAT_D24_UNORM_S8_UINT)
        dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
    else
        dsTexFormat = DXGI_FORMAT_R32_TYPELESS;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = 1;
	desc.BindFlags = bindFlags;
	desc.CPUAccessFlags = 0;
	desc.Format = dsTexFormat;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = multiSamples;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;
	DXCall(device->CreateTexture2D(&desc, NULL, &Texture));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = format;
    dsvDesc.ViewDimension = multiSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = 0;
	DXCall(device->CreateDepthStencilView(Texture, &dsvDesc, &DSView));

    if (useAsShaderResource)
    {
        DXGI_FORMAT dsSRVFormat;
        if (format == DXGI_FORMAT_D16_UNORM)
            dsSRVFormat = DXGI_FORMAT_R16_UNORM;
        else if(format == DXGI_FORMAT_D24_UNORM_S8_UINT)
            dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS ;
        else
            dsSRVFormat = DXGI_FORMAT_R32_FLOAT;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = dsSRVFormat;
        srvDesc.ViewDimension = multiSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
	    DXCall(device->CreateShaderResourceView(Texture, &srvDesc, &SRView));
    }
    else
        SRView = NULL;

	Width = width;
	Height = height;
	MultiSamples = multiSamples;
	Format = format;
}

}