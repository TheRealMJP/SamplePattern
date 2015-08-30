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
#include "GUIObject.h"
#include "SpriteFont.h"

namespace SampleFramework11
{

class Slider : public GUIObject
{

public:



    Slider();
    ~Slider();

    void Initalize(ID3D11Device* device);

    void Update(UINT mousePosX, UINT mousePosY, bool mouseLButtonDown);
    void Render(SpriteRenderer& renderer);


    // XMFLOAT2& Size() { return size; };
    float& MinVal() { return minVal; };
    float& MaxVal() { return maxVal; };
    UINT& NumSteps() { return numSteps; };
    float& Value() { return value; };
    std::wstring& Name() { return name; };

    // XMFLOAT2 Size() const { return size; };
    float MinVal() const { return minVal; };
    float MaxVal() const { return maxVal; };
    UINT NumSteps() const { return numSteps; };
    float Value() const { return value; };
    std::wstring Name() const { return name; };

protected:

    static const UINT TextureWidth = 512;
    static const UINT TextureHeight = 64;

    XMFLOAT2 size;
    float minVal;
    float maxVal;
    UINT numSteps;
    float value;
    std::wstring name;
    SpriteFont font;
    bool dragging;
    float dragValue;
    bool hover;
    bool mouseDown;
    XMFLOAT2 dragPos;

    ID3D11ShaderResourceViewPtr barTexture;
    ID3D11ShaderResourceViewPtr knobTexture;

    XMFLOAT4 GetKnobBounds();
    void CalcValue(float normalizedValue);
};

}