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
#include "GraphicsTypes.h"

namespace SampleFramework11
{

class DeviceManager
{

public:

	DeviceManager();
	~DeviceManager();

	void Initialize(HWND outputWindow);
	void Reset();
	void Present();

	// Getters
	ID3D11Device*				Device() const				    { return device.GetInterfacePtr(); };
	ID3D11DeviceContext*		ImmediateContext() const		{ return immediateContext.GetInterfacePtr(); };
	IDXGISwapChain*				SwapChain() const			    { return swapChain.GetInterfacePtr(); };
	ID3D11RenderTargetView*		BackBuffer() const			    { return bbRTView.GetInterfacePtr(); };
	ID3D11DepthStencilView*		AutoDepthStencilView() const	{ return autoDSView; };
    ID3D11ShaderResourceView*   AutoDepthStencilSRView() const  { return autoDSSRView; };
	D3D_FEATURE_LEVEL			FeatureLevel() const			{ return featureLevel; };

	DXGI_FORMAT		BackBufferFormat() const		{ return backBufferFormat; };
	UINT			BackBufferWidth() const			{ return backBufferWidth; };
	UINT			BackBufferHeight() const		{ return backBufferHeight; };
	UINT			BackBufferMSCount() const		{ return msCount; };
	UINT			BackBufferMSQuality() const		{ return msQuality; };
	bool			AutoDepthStencilEnabled() const	{ return enableAutoDS; };
	DXGI_FORMAT		AutoDepthStencilFormat() const	{ return autoDSFormat; };
    bool            UseAutoDepthStencilAsSR() const { return useAutoDSAsSR; };
	bool			FullScreen() const		{ return fullScreen; };
	bool			VSYNCEnabled() const			{ return vsync; };


	// Setters
	void SetBackBufferFormat(DXGI_FORMAT format)	{ backBufferFormat = format; };
	void SetBackBufferWidth(UINT width)				{ backBufferWidth = width; };
	void SetBackBufferHeight(UINT height)			{ backBufferHeight = height; };
	void SetBackBufferMSCount(UINT count)			{ msCount = count; };
	void SetBackBufferMSQuality(UINT quality)		{ msQuality = quality; };
	void SetAutoDepthStencilEnabled(bool enabled)	{ enableAutoDS = enabled; };
	void SetAutoDepthStencilFormat(DXGI_FORMAT fmt)	{ autoDSFormat = fmt; };
    void SetUseAutoDSAsSR(bool useAsSR)             { useAutoDSAsSR = useAsSR; };
	void SetFullScreen(bool enabled)			{ fullScreen = enabled; };
	void SetVSYNCEnabled(bool enabled)				{ vsync = enabled; };

protected:

    void CheckForSuitableOutput();
    void AfterReset();
    void PrepareFullScreenSettings();

    IDXGIFactory1Ptr                factory;
    IDXGIAdapter1Ptr                adapter;
    IDXGIOutputPtr                  output;

	ID3D11DevicePtr					device;
	ID3D11DeviceContextPtr			immediateContext;
	IDXGISwapChainPtr				swapChain;
	ID3D11Texture2DPtr				bbTexture;
	ID3D11RenderTargetViewPtr		bbRTView;

	DXGI_FORMAT					backBufferFormat;
	UINT						backBufferWidth;
	UINT						backBufferHeight;
	UINT						msCount;
	UINT						msQuality;
	bool						enableAutoDS;
	ID3D11Texture2DPtr			autoDSTexture;
    ID3D11DepthStencilViewPtr   autoDSView;
	DXGI_FORMAT					autoDSFormat;
    ID3D11ShaderResourceViewPtr autoDSSRView;
    bool                        useAutoDSAsSR;
	bool						fullScreen;
	bool						vsync;
    DXGI_RATIONAL               refreshRate;

	D3D_FEATURE_LEVEL			featureLevel;
};

}