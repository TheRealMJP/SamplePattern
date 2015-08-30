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

#include "Window.h"
#include "DeviceManager.h"
#include "DeviceStates.h"
#include "Timer.h"

namespace SampleFramework11
{

class App
{

public:

    App(LPCWSTR appName, LPCWSTR iconResource = NULL);
    virtual ~App();

    void Run();

protected:

    virtual void Initialize();
    virtual void LoadContent();
    virtual void Update(const Timer& timer) = 0;
    virtual void Render(const Timer& timer) = 0;

    virtual void BeforeReset();
    LRESULT WindowResized(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void AfterReset();

    void Exit();
    void ToggleFullScreen(bool fullScreen);

    Window window;
    DeviceManager deviceManager;
    Timer timer;

    BlendStates blendStates;
    RasterizerStates rasterizerStates;
    DepthStencilStates depthStencilStates;
    SamplerStates samplerStates;
};

}