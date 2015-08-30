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
#include "Utility.h"

namespace SampleFramework11
{

struct RenderTarget2D
{
	ID3D11Texture2DPtr Texture;
	ID3D11RenderTargetViewPtr RTView;
	ID3D11ShaderResourceViewPtr SRView;
	UINT Width;
	UINT Height;
	UINT NumMipLevels;
	UINT MultiSamples;
	UINT MSQuality;
	DXGI_FORMAT Format;
	bool AutoGenMipMaps;
    std::vector<ID3D11RenderTargetViewPtr> MipLevels;

    RenderTarget2D();

	void Initialize(    ID3D11Device* device,
						UINT width,
						UINT height,
						DXGI_FORMAT format,
						UINT numMipLevels = 1,
						UINT multiSamples = 1,
						UINT msQuality = 0,
						bool autoGenMipMaps = false);
};

struct DepthStencilBuffer
{
	ID3D11Texture2DPtr Texture;
	ID3D11DepthStencilViewPtr DSView;
	ID3D11ShaderResourceViewPtr SRView;
	UINT Width;
	UINT Height;
	UINT MultiSamples;
	UINT MSQuality;
	DXGI_FORMAT Format;

    DepthStencilBuffer();

	void Initialize(	ID3D11Device* device,
						UINT width,
						UINT height,
						DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT,
						bool useAsShaderResource = false,
						UINT multiSamples = 1,
						UINT msQuality = 0);
};

template<typename T> class ConstantBuffer
{
public:

    T Data;

protected:

    ID3D11BufferPtr buffer;
    bool initialized;

public:

    ConstantBuffer() : initialized(false)
    {
        ZeroMemory(&Data, sizeof(T));
    }

    ID3D11Buffer* Buffer() const
    {
        return buffer;
    }

    void Initialize(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC desc;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)));

        DXCall(device->CreateBuffer(&desc, NULL, &buffer));

        initialized = true;
    }

    void ApplyChanges(ID3D11DeviceContext* deviceContext)
    {
        _ASSERT(initialized);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DXCall(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        CopyMemory(mappedResource.pData, &Data, sizeof(T));
        deviceContext->Unmap(buffer, 0);
    }

    void SetVS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->VSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetPS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->PSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetGS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->GSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetHS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->HSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetDS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->DSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetCS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->CSSetConstantBuffers(slot, 1, bufferArray);
    }
};

// For aligning to float4 boundaries
#define Float4Align __declspec(align(32))

class PIXEvent
{
public:

    PIXEvent(const WCHAR* markerName)
    {
        int retVal = D3DPERF_BeginEvent(0xFFFFFFFF, markerName);
        //_ASSERT(retVal >= 0);
    }

    ~PIXEvent()
    {
       int retVal = D3DPERF_EndEvent();
       //_ASSERT(retVal >= 0);
    }
};

}