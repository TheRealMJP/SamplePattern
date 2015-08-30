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

#include "Slider.h"

#include "Utility.h"

namespace SampleFramework11
{

Slider::Slider() :  size(100.0f, 25.0f),
                    minVal(0.0f),
                    maxVal(100.0f),
                    numSteps(100),
                    value(0),
                    dragging(false),
                    hover(false),
                    dragValue(0),
                    mouseDown(false)
{
}

Slider::~Slider()
{
}

void Slider::Initalize(ID3D11Device* device)
{
    // Load the texures
    D3DX11_IMAGE_LOAD_INFO info;
    info.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    info.Width = D3DX11_DEFAULT;
    info.Height = D3DX11_DEFAULT;
    info.Depth = D3DX11_DEFAULT;
    info.FirstMipLevel = D3DX11_DEFAULT;
    info.MipLevels = D3DX11_DEFAULT;
    info.Usage = (D3D11_USAGE) D3DX11_DEFAULT;
    info.BindFlags = D3DX11_DEFAULT;
    info.CpuAccessFlags = D3DX11_DEFAULT;
    info.MiscFlags = D3DX11_DEFAULT;
    info.Format = DXGI_FORMAT_FROM_FILE;
    info.Filter = D3DX11_DEFAULT;
    info.MipFilter = D3DX11_DEFAULT;
    info.pSrcInfo = NULL;

    DXCall(D3DX11CreateShaderResourceViewFromFileW(device, L"SampleFramework11\\Images\\SliderBar.png", &info, NULL, &barTexture, NULL));
    DXCall(D3DX11CreateShaderResourceViewFromFileW(device, L"SampleFramework11\\Images\\SliderKnob.png", &info, NULL, &knobTexture, NULL));

    font.Initialize(L"Microsoft Sans Serif", 8.5f, SpriteFont::Regular, true, device);

    enabled = true;
}

void Slider::CalcValue(float normalizedValue)
{
    float stepsValue = normalizedValue * numSteps;
    stepsValue = std::floorf(stepsValue);
    normalizedValue = stepsValue / numSteps;
    value = minVal + ((maxVal - minVal) * normalizedValue);
}

void Slider::Update(UINT mousePosX, UINT mousePosY, bool mouseLButtonDown)
{
    if(!enabled)
    {
        dragging = false;
        mouseDown = false;
        hover = false;
        return;
    }

    // Figure out the x value of the mouse relative to the slider
    float mouseX = mousePosX - position.x;
    float mouseY = mousePosY - position.y;

    // Figure out if we're over the knob
    XMFLOAT4 knobBounds = GetKnobBounds();
    float left = knobBounds.x - position.x;
    float top = knobBounds.y - position.y;
    float right = left + knobBounds.z * TextureHeight;
    float bottom = top + knobBounds.w * TextureHeight;
    bool knobHover = mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom;
    hover = knobHover;

    if (mouseLButtonDown)
    {
        bool clicked = !mouseDown;
        mouseDown = true;

        if (clicked && knobHover && !dragging)
        {
            dragPos = XMFLOAT2(mouseX, mouseY);
            dragging = true;
            dragValue = value;
        }
        else if (dragging)
        {
            float offset = (dragValue - minVal) / (maxVal - minVal);
            float normalizedValue = Clamp(offset + ((mouseX - dragPos.x) / size.x), 0, 1);
            CalcValue(normalizedValue);

        }
    }
    else
    {
        mouseDown = false;
        dragging = false;
    }
}

XMFLOAT4 Slider::GetKnobBounds()
{
    float normalizedValue = (value - minVal) / (maxVal - minVal);
    float scaleX = size.y / TextureHeight;
    float scaleY = size.y / TextureHeight;
    float posX = position.x + (normalizedValue * size.x) - (size.y / 2.0f);
    float posY = position.y - (size.y / 2.5f);
    return XMFLOAT4(posX, posY, scaleX, scaleY);
}

void Slider::Render(SpriteRenderer& renderer)
{
    value = Clamp(value, minVal, maxVal);

    float alpha = enabled ? 1.0f : 0.35f;

    // Render the bar
    XMMATRIX transform = XMMatrixScaling(size.x / TextureWidth, size.y / TextureHeight, 1.0f);
    transform._41 = position.x;
    transform._42 = position.y;
    renderer.Render(barTexture, transform, XMFLOAT4(1, 1, 1, alpha));

    // Render the knob
    XMFLOAT4 knobBounds = GetKnobBounds();
    transform = XMMatrixScaling(knobBounds.w, knobBounds.z, 1.0f);
    transform._41 = knobBounds.x;
    transform._42 = knobBounds.y;
    XMFLOAT4 knobColor = hover ? XMFLOAT4(1.5f, 1.5f, 1.5f, alpha) : XMFLOAT4(1, 1, 1, alpha);
    renderer.Render(knobTexture, transform, knobColor);

    transform = XMMatrixTranslation(position.x, position.y + size.y - 12.0f, 0);
    renderer.RenderText(font, name.c_str(), transform, XMFLOAT4(1, 1, 1, alpha));

    std::wstring valString = ToString(value);
    transform = XMMatrixTranslation(position.x + size.x + 16.0f, position.y - 4.0f, 0);
    renderer.RenderText(font, valString.c_str(), transform, XMFLOAT4(1, 1, 1, alpha));
}

}
