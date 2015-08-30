//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "ShaderCompilation.h"

#include "Utility.h"
#include "Exceptions.h"
#include "InterfacePointers.h"

namespace SampleFramework11
{

ID3D10Blob* CompileShader(LPCWSTR path,
                            LPCSTR functionName,
                            LPCSTR profile,
                            CONST D3D10_SHADER_MACRO* defines,
                            ID3D10Include* includes)
{
    // Loop until we succeed, or an exception is thrown
    while (true)
    {

        UINT flags = 0;
#ifdef _DEBUG
        flags |= D3D10_SHADER_DEBUG|D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

        ID3D10Blob* compiledShader;
        ID3D10BlobPtr errorMessages;
        HRESULT hr = D3DX11CompileFromFileW(path, defines, includes, functionName, profile,
            flags, 0, NULL, &compiledShader, &errorMessages, NULL);

        if (FAILED(hr))
        {
            if (errorMessages)
            {
                WCHAR message[1024];
                message[0] = NULL;
                char* blobdata = reinterpret_cast<char*>(errorMessages->GetBufferPointer());

                MultiByteToWideChar(CP_ACP, 0, blobdata, static_cast<int>(errorMessages->GetBufferSize()), message, 1024);
                std::wstring fullMessage = L"Error compiling shader file \"";
                fullMessage += path;
                fullMessage += L"\" - ";
                fullMessage += message;

#ifdef _DEBUG
                // Pop up a message box allowing user to retry compilation
                int retVal = MessageBoxW(NULL, fullMessage.c_str(), L"Shader Compilation Error", MB_RETRYCANCEL);
                if(retVal != IDRETRY)
                    throw DXException(hr, fullMessage.c_str());
#else
                throw DXException(hr, fullMessage.c_str());
#endif
            }
            else
            {
                _ASSERT(false);
                throw DXException(hr);
            }
        }
        else
            return compiledShader;
    }
}

ID3D11VertexShader* CompileVSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D10_SHADER_MACRO* defines,
                                        ID3D10Include* includes,
                                        ID3D10Blob** byteCode)
{
    ID3D10BlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11VertexShader* shader = NULL;
    DXCall(device->CreateVertexShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    if (byteCode != NULL)
        *byteCode = compiledShader.Detach();

    return shader;
}

ID3D11PixelShader* CompilePSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D10_SHADER_MACRO* defines,
                                        ID3D10Include* includes)
{
    ID3D10BlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11PixelShader* shader = NULL;
    DXCall(device->CreatePixelShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11GeometryShader* CompileGSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D10_SHADER_MACRO* defines,
                                        ID3D10Include* includes)
{
    ID3D10BlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11GeometryShader* shader = NULL;
    DXCall(device->CreateGeometryShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11HullShader* CompileHSFromFile(ID3D11Device* device,
                                    LPCWSTR path,
                                    LPCSTR functionName,
                                    LPCSTR profile,
                                    CONST D3D10_SHADER_MACRO* defines,
                                    ID3D10Include* includes)
{
    ID3D10BlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11HullShader* shader = NULL;
    DXCall(device->CreateHullShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11DomainShader* CompileDSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D10_SHADER_MACRO* defines,
                                        ID3D10Include* includes)
{
    ID3D10BlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11DomainShader* shader = NULL;
    DXCall(device->CreateDomainShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11ComputeShader* CompileCSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D10_SHADER_MACRO* defines,
                                        ID3D10Include* includes)
{
    ID3D10BlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11ComputeShader* shader = NULL;
    DXCall(device->CreateComputeShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

}