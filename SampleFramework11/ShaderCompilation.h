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

namespace SampleFramework11
{

// Compiles a shader from file and returns the compiled bytecode
ID3D10Blob* CompileShader(LPCWSTR path,
                            LPCSTR functionName,
                            LPCSTR profile,
                            CONST D3D10_SHADER_MACRO* defines = NULL,
                            ID3D10Include* includes = NULL);

// Compiles a shader from file and creates the appropriate shader instance
ID3D11VertexShader* CompileVSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "VS",
                                        LPCSTR profile = "vs_4_0",
                                        CONST D3D10_SHADER_MACRO* defines = NULL,
                                        ID3D10Include* includes = NULL,
                                        ID3D10Blob** byteCode = NULL);

ID3D11PixelShader* CompilePSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "PS",
                                        LPCSTR profile = "ps_4_0",
                                        CONST D3D10_SHADER_MACRO* defines = NULL,
                                        ID3D10Include* includes = NULL);

ID3D11GeometryShader* CompileGSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "GS",
                                        LPCSTR profile = "gs_4_0",
                                        CONST D3D10_SHADER_MACRO* defines = NULL,
                                        ID3D10Include* includes = NULL);

ID3D11HullShader* CompileHSFromFile(ID3D11Device* device,
                                    LPCWSTR path,
                                    LPCSTR functionName = "HS",
                                    LPCSTR profile = "hs_5_0",
                                    CONST D3D10_SHADER_MACRO* defines = NULL,
                                    ID3D10Include* includes = NULL);

ID3D11DomainShader* CompileDSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "DS",
                                        LPCSTR profile = "ds_5_0",
                                        CONST D3D10_SHADER_MACRO* defines = NULL,
                                        ID3D10Include* includes = NULL);

ID3D11ComputeShader* CompileCSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "CS",
                                        LPCSTR profile = "cs_4_0",
                                        CONST D3D10_SHADER_MACRO* defines = NULL,
                                        ID3D10Include* includes = NULL);

}