//======================================================================
//
//	MSAA Sample Pattern Inspector
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#pragma once

#include "SampleFramework11/PCH.h"

#include "SampleFramework11/App.h"
#include "SampleFramework11/InterfacePointers.h"
#include "SampleFramework11/Camera.h"
#include "SampleFramework11/Model.h"
#include "SampleFramework11/SpriteFont.h"
#include "SampleFramework11/SpriteRenderer.h"
#include "SampleFramework11/Skybox.h"
#include "SampleFramework11/GraphicsTypes.h"
#include "SampleFramework11/Slider.h"

using namespace SampleFramework11;

class SamplePattern : public App
{

protected:       

    SpriteFont font;
    SpriteFont smallFont;
    SpriteRenderer spriteRenderer;    

    RenderTarget2D sampleTarget;    
    RenderTarget2D patternTarget;
    ID3D11Texture2DPtr stagingTexture;

    ID3D11ShaderResourceViewPtr whiteTexture;
    
    UINT currMSAAMode;
    ID3D11PixelShaderPtr patternDetectShaders[D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT];
    std::vector<DXGI_SAMPLE_DESC> msaaModes;

	ID3D11RasterizerStatePtr msaa2xRState;
	ID3D11RasterizerStatePtr msaa4xRState;
	ID3D11RasterizerStatePtr msaa8xRState;

	bool useCustomSampling;
	bool nvExtensionsAvailable;

	struct PatternDetectConstants
	{
		UINT PixelPosX;
		UINT PixelPosY;
	};

	ConstantBuffer<PatternDetectConstants> patternDetectConstants;
        
    virtual void LoadContent();
    virtual void Render(const Timer& timer);
    virtual void Update(const Timer& timer);
    virtual void BeforeReset();
    virtual void AfterReset();
    
    void SetupMSAAMode();
        
    void RenderHUD();

public:

    SamplePattern();    
};

