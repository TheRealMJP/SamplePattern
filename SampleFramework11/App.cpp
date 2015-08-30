//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "App.h"
#include "Exceptions.h"

using std::tr1::bind;
using std::tr1::mem_fn;
using namespace std::tr1::placeholders;

namespace SampleFramework11
{

App::App(LPCWSTR appName, LPCWSTR iconResource) :  window(NULL, appName, WS_OVERLAPPEDWINDOW,
                                                            WS_EX_APPWINDOW, 1280, 720, iconResource, iconResource)
{

}

App::~App()
{

}

void App::Run()
{
    try
    {
        Initialize();

        window.ShowWindow();

        deviceManager.Initialize(window);

        blendStates.Initialize(deviceManager.Device());
        rasterizerStates.Initialize(deviceManager.Device());
        depthStencilStates.Initialize(deviceManager.Device());
        samplerStates.Initialize(deviceManager.Device());

        window.SetUserMessageFunction(WM_SIZE, bind(mem_fn(&App::WindowResized), this, _1, _2, _3, _4));

        LoadContent();

        AfterReset();

        while(window.IsAlive())
        {
            if(!window.IsMinimized())
            {
                timer.Update();

                Update(timer);

                Render(timer);
                deviceManager.Present();
            }

            window.MessageLoop();
        }
    }
    catch (SampleFramework11::Exception exception)
    {
        exception.ShowErrorMessage();
    }
}

LRESULT App::WindowResized(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(!deviceManager.FullScreen() && wParam != SIZE_MINIMIZED)
    {
        int width, height;
        window.GetClientArea(width, height);

        if(width != deviceManager.BackBufferWidth() || height != deviceManager.BackBufferHeight())
        {
            BeforeReset();

            deviceManager.SetBackBufferWidth(width);
            deviceManager.SetBackBufferHeight(height);
            deviceManager.Reset();

            AfterReset();
        }
    }

    return 0;
}

void App::Exit()
{
    window.Destroy();
}

void App::Initialize()
{
}

void App::LoadContent()
{
}

void App::BeforeReset()
{
}

void App::AfterReset()
{
}

void App::ToggleFullScreen(bool fullScreen)
{
    if(fullScreen != deviceManager.FullScreen())
    {
        BeforeReset();

        deviceManager.SetFullScreen(fullScreen);
        deviceManager.Reset();

        AfterReset();
    }
}

}