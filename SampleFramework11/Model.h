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

namespace SampleFramework11
{

struct MeshMaterial
{
    XMFLOAT3 AmbientAlbedo;
    XMFLOAT3 DiffuseAlbedo;
    XMFLOAT3 SpecularAlbedo;
    XMFLOAT3 Emissive;
    float SpecularPower;
    float Alpha;
    std::wstring DiffuseMapName;
    std::wstring NormalMapName;
    ID3D11ShaderResourceView* DiffuseMap;
    ID3D11ShaderResourceView* NormalMap;
};

struct MeshPart
{
    UINT VertexStart;
    UINT VertexCount;
    UINT IndexStart;
    UINT IndexCount;
    UINT MaterialIdx;
};

class Mesh
{
    friend class Model;

public:

    enum IndexType
    {
        Index16Bit = 0,
        Index32Bit = 1
    };

    // Lifetime
    Mesh();
    ~Mesh();

    // Rendering
    void Render(ID3D11DeviceContext* context);

    // Accessors
    ID3D11Buffer* VertexBuffer() { return vertexBuffer; };
    const ID3D11Buffer* VertexBuffer() const { return vertexBuffer; };
    ID3D11Buffer* IndexBuffer() { return indexBuffer; };
    const ID3D11Buffer* IndexBuffer() const { return indexBuffer; };

    std::vector<MeshPart>& MeshParts() { return meshParts; };
    const std::vector<MeshPart>& MeshParts() const { return meshParts; };

    const D3D11_INPUT_ELEMENT_DESC* InputElements() const { return &inputElements[0]; };
    UINT NumInputElements() const { return static_cast<UINT>(inputElements.size()); };

    DWORD VertexStride() const { return vertexStride; };
    DWORD NumVertices() const { return numVertices; };
    DWORD NumIndices() const { return numIndices; };

    IndexType IndexBufferType() const { return indexType; };
    DXGI_FORMAT IndexBufferFormat() const { return indexType == Index32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT; };

protected:

    void CreateFromD3DXMesh(const std::wstring& directory, ID3D11Device* device,
                            IDirect3DDevice9* d3d9Device, ID3DXMesh* mesh, bool generateNormals,
                            bool GenerateTangentFrame, DWORD* initalAdjacency, IndexType idxType);

    ID3DXMesh* GenerateTangentFrame(ID3DXMesh* mesh, IDirect3DDevice9* d3d9Device);
    ID3DXMesh* GenerateNormals(ID3DXMesh* mesh, IDirect3DDevice9* d3d9Device);
    void CreateInputElements(D3DVERTEXELEMENT9* declaration);

    ID3D11BufferPtr vertexBuffer;
    ID3D11BufferPtr indexBuffer;

    std::vector<MeshPart> meshParts;
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

    XMFLOAT3 bBoxMin;
    XMFLOAT3 bBoxMax;

    XMFLOAT3 bSphereCenter;
    float bSphereRadius;

    std::vector<DWORD> adjacency;
    DWORD vertexStride;
    DWORD numVertices;
    DWORD numIndices;

    IndexType indexType;
};

class Model
{
public:

    // Constructor/Destructor
    Model();
    ~Model();

    // Loading from file formats
    void CreateFromXFile(ID3D11Device* device,
                        LPCWSTR fileName,
                        const WCHAR* normalMapSuffix = NULL,
                        bool generateNormals = false,
                        bool generateTangentFrame = false,
                        Mesh::IndexType idxType = Mesh::Index16Bit);

    void CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName);

    // Accessors
    std::vector<MeshMaterial>& Materials() { return meshMaterials; };
    const std::vector<MeshMaterial>& Materials() const { return meshMaterials; };

    std::vector<Mesh>& Meshes() { return meshes; }
    const std::vector<Mesh>& Meshes() const { return meshes; }

protected:

    static IDirect3DDevice9Ptr CreateD3D9Device();
    static void LoadMaterialResources(MeshMaterial& material, const std::wstring& directory, ID3D11Device* device);

    std::vector<Mesh> meshes;
    std::vector<MeshMaterial> meshMaterials;
};

}