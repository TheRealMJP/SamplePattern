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

namespace SampleFramework11
{

// Constants
const float Pi = 3.14159f;
const float PiOver2 = Pi / 2.0f;
const float PiOver4 = Pi / 4.0f;

// Throws a DXException on failing HRESULT
inline void DXCall(HRESULT hr)
{
	if (FAILED(hr))
    {
        _ASSERT(false);
		throw DXException(hr);
    }
}

// Throws a Win32Exception on failing return value
inline void Win32Call(BOOL retVal)
{
    if (retVal == 0)
	{
        _ASSERT(false);
        throw Win32Exception(GetLastError());
    }
}

// Throws a GdiPlusException on failing Status value
inline void GdiPlusCall(Gdiplus::Status status)
{
    if (status != Gdiplus::Ok)
    {
        _ASSERT(false);
        throw GdiPlusException(status);
    }
}

// Returns a size suitable for creating a constant buffer, by rounding up
// to the next multiple of 16
inline UINT CBSize(UINT size)
{
    UINT cbsize = size + (16 - (size % 16));
    return cbsize;
}

// Returns the forward vector from a transform matrix
inline XMVECTOR ForwardVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._31, matrix._32, matrix._33));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR BackVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(-matrix._31, -matrix._32, -matrix._33));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR RightVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._11, matrix._12, matrix._13));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR LeftVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(-matrix._11, -matrix._12, -matrix._13));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR UpVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._21, matrix._22, matrix._23));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR DownVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(-matrix._21, -matrix._22, -matrix._23));
}

// Returns the translation vector from a transform matrix
inline XMVECTOR TranslationVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._41, matrix._42, matrix._43));
}

// Sets the translation vector in a transform matrix
inline void SetTranslationVec(XMMATRIX& matrix, const XMVECTOR& translation)
{
    matrix._41 = XMVectorGetX(translation);
    matrix._42 = XMVectorGetY(translation);
    matrix._43 = XMVectorGetZ(translation);
}

// Clamps a value to the specified range
inline float Clamp(float val, float min, float max)
{
    _ASSERT(max >= min);

    if (val < min)
        val = min;
    else if (val > max)
        val = max;
    return val;
}

inline XMFLOAT2 Clamp(const XMFLOAT2& val, const XMFLOAT2 min, const XMFLOAT2 max)
{
    XMFLOAT2 retVal;
    retVal.x = Clamp(val.x, min.x, max.x);
    retVal.y = Clamp(val.y, min.y, max.y);
}

inline XMFLOAT3 Clamp(const XMFLOAT3& val, const XMFLOAT3 min, const XMFLOAT3 max)
{
    XMFLOAT3 retVal;
    retVal.x = Clamp(val.x, min.x, max.x);
    retVal.y = Clamp(val.y, min.y, max.y);
    retVal.z = Clamp(val.z, min.z, max.z);
}

inline XMFLOAT4 Clamp(const XMFLOAT4& val, const XMFLOAT4 min, const XMFLOAT4 max)
{
    XMFLOAT4 retVal;
    retVal.x = Clamp(val.x, min.x, max.x);
    retVal.y = Clamp(val.y, min.y, max.y);
    retVal.z = Clamp(val.z, min.z, max.z);
    retVal.w = Clamp(val.w, min.w, max.w);
}

// Converts an ANSI string to a std::wstring
inline std::wstring AnsiToWString(const char* ansiString)
{
    WCHAR buffer[512];
    Win32Call(MultiByteToWideChar(CP_ACP, 0, ansiString, -1, buffer, 512));
    return std::wstring(buffer);
}

// Returns true if a file exits
inline bool FileExists(const WCHAR* fileName)
{
    _ASSERT(fileName);

    DWORD fileAttr;
    fileAttr = GetFileAttributes(fileName);
    if (fileAttr == INVALID_FILE_ATTRIBUTES)
        return false;

    return true;
}

// Returns the directory containing a file
inline std::wstring GetDirectoryFromFileName(const WCHAR* fileName)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'\\');
    if (idx != std::wstring::npos)
        return filePath.substr(0, idx + 1);
    else
        return std::wstring(L"");
}

// Returns the given filename, minus the extension
inline std::wstring GetFileNameWithoutExtension(const WCHAR* fileName)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'.');
    if (idx != std::wstring::npos)
        return filePath.substr(0, idx);
    else
        return std::wstring(L"");
}


// Returns the extension of the filename
inline std::wstring GetFileExtension(const WCHAR* fileName)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'.');
    if (idx != std::wstring::npos)
        return filePath.substr(idx + 1, filePath.length() - idx - 1);
    else
        return std::wstring(L"");
}

// Splits up a string using a delimiter
inline void Split(const std::wstring& str, std::vector<std::wstring>& parts, const std::wstring& delimiters = L" ")
{
    // Skip delimiters at beginning
    std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter"
    std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::wstring::npos != pos || std::wstring::npos != lastPos)
    {
        // Found a token, add it to the vector
        parts.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// Splits up a string using a delimiter
inline std::vector<std::wstring> Split(const std::wstring& str, const std::wstring& delimiters = L" ")
{
    std::vector<std::wstring> parts;
    Split(str, parts, delimiters);
    return parts;
}

// Parses a string into a number
template<typename T> inline T Parse(const std::wstring& str)
{
    std::wistringstream stream(str);
    wchar_t c;
    T x;
    if (!(str >> x) || stream.get(c))
        throw Exception(L"Can't parse string \"" + str + L"\"");
    return x;
}

// Converts a number to a string
template<typename T> inline std::wstring ToString(const T& val)
{
    std::wostringstream stream;
    if (!(stream << val))
        throw Exception(L"Error converting value to string");
    return stream.str();
}

// Returns the number of mip levels given a texture size
inline UINT NumMipLevels(UINT width, UINT height)
{
    UINT numMips = 0;
    UINT size = max(width, height);
    while (1U << numMips <= size)
        ++numMips;

    if (1U << numMips < size)
        ++numMips;

    return numMips;
}

template<typename T>
class ArrayDeleter
{
public:
    ArrayDeleter(T* ptr)
    {
        _ASSERT(ptr);
        array = ptr;
    }

    ~ArrayDeleter() { delete[] array; }

protected:
    T* array;
};

template<typename T>
class IUnknownReleaser
{
public:
    IUnknownReleaser(T* ptr)
    {
        _ASSERT(ptr);
        this->ptr = ptr;
    }

    ~IUnknownReleaser() { ptr->Release(); }

protected:
    T* ptr;
};

}