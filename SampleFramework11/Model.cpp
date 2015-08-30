//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "Model.h"

#include "SDKmesh.h"
#include "Exceptions.h"
#include "Utility.h"

using std::string;
using std::wstring;
using std::vector;
using std::map;
using std::wifstream;

namespace SampleFramework11
{

Mesh::Mesh() :  vertexStride(0),
                numVertices(0),
                numIndices(0)
{
}

Mesh::~Mesh()
{
}

void Mesh::CreateFromD3DXMesh(const wstring& directory, ID3D11Device* device, IDirect3DDevice9* d3d9Device,
                                ID3DXMesh* mesh, bool generateNormals, bool generateTangentFrame,
                                DWORD* initalAdjacency, IndexType idxType)
{
    indexType = idxType;

    // Optimize the mesh
    adjacency.resize(mesh->GetNumFaces() * 3, 0);
    DXCall(mesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_ATTRSORT,
            initalAdjacency, &adjacency[0], NULL, NULL));

    if (generateTangentFrame)
        mesh = GenerateTangentFrame(mesh, d3d9Device);
    else if (generateNormals)
        mesh = GenerateNormals(mesh, d3d9Device);

    // Get some of the mesh info
    vertexStride = mesh->GetNumBytesPerVertex();
    numVertices = mesh->GetNumVertices();
    numIndices = mesh->GetNumFaces() * 3;

    // Convert the D3D9 vertex declaration to a D3D11 input element desc
    D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
    DXCall(mesh->GetDeclaration(declaration));
    CreateInputElements(declaration);

    // Copy over the vertex data
    void* vertices = NULL;
    DXCall(mesh->LockVertexBuffer(0, &vertices));

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = vertexStride * numVertices;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = vertices;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    DXCall(device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer));

    // Compute bounding box and sphere
    D3DXVECTOR3* boxMin = reinterpret_cast<D3DXVECTOR3*>(&bBoxMin);
    D3DXVECTOR3* boxMax = reinterpret_cast<D3DXVECTOR3*>(&bBoxMax);
    DXCall(D3DXComputeBoundingBox(reinterpret_cast<D3DXVECTOR3*>(vertices), numVertices, vertexStride, boxMin, boxMax));

    D3DXVECTOR3* sphereCenter = reinterpret_cast<D3DXVECTOR3*>(&bSphereCenter);
    DXCall(D3DXComputeBoundingSphere(reinterpret_cast<D3DXVECTOR3*>(vertices), numVertices, vertexStride, sphereCenter, &bSphereRadius));

    DXCall(mesh->UnlockVertexBuffer());

    // Copy over the index data
    void* indices = NULL;
    DXCall(mesh->LockIndexBuffer(0, &indices));

    UINT indexSize = indexType == Index32Bit ? 4 : 2;
    bufferDesc.ByteWidth = indexSize * numIndices;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    initData.pSysMem = indices;
    DXCall(device->CreateBuffer(&bufferDesc, &initData, &indexBuffer));

    DXCall(mesh->UnlockIndexBuffer());

    // Copy in the subset info
    DWORD numSubsets = 0;
    DXCall(mesh->GetAttributeTable(NULL, &numSubsets));
    D3DXATTRIBUTERANGE* attributeTable = new D3DXATTRIBUTERANGE[numSubsets];
    ArrayDeleter<D3DXATTRIBUTERANGE> tableDeleter(attributeTable);
    DXCall(mesh->GetAttributeTable(attributeTable, &numSubsets));
    for(UINT i = 0; i < numSubsets; ++i)
    {
        MeshPart part;
        part.VertexStart = attributeTable[i].VertexStart;
        part.VertexCount = attributeTable[i].VertexCount;
        part.IndexStart = attributeTable[i].FaceStart * 3;
        part.IndexCount = attributeTable[i].FaceCount * 3;
        part.MaterialIdx = attributeTable[i].AttribId;
        meshParts.push_back(part);
    }
}

ID3DXMesh* Mesh::GenerateTangentFrame(ID3DXMesh* mesh, IDirect3DDevice9* d3d9Device)
{
    // make sure we have a texture coordinate
    D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
    DXCall(mesh->GetDeclaration(decl));
    bool foundTexCoord = false;
    for (UINT i = 0; i < MAX_FVF_DECL_SIZE; ++i)
    {
        if (decl[i].Stream == 0xFF)
            break;
        else if(decl[i].Usage == D3DDECLUSAGE_TEXCOORD && decl[i].UsageIndex == 0)
        {
            foundTexCoord = true;
            break;
        }
    }

    _ASSERT(foundTexCoord);

    // Clone the mesh with a new declaration
    D3DVERTEXELEMENT9 newDecl[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },
        { 0, 44, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,  0 },
        D3DDECL_END()
    };

    ID3DXMesh* clonedMesh = NULL;

    UINT options = D3DXMESH_MANAGED;
    if (indexType == Index32Bit)
        options |= D3DXMESH_32BIT;
    DXCall(mesh->CloneMesh(options, newDecl, d3d9Device, &clonedMesh));
    mesh->Release();

    // Calculate the tangent frame
    DXCall(D3DXComputeTangentFrameEx(clonedMesh,
        D3DDECLUSAGE_TEXCOORD, 0,
        D3DDECLUSAGE_BINORMAL, 0,
        D3DDECLUSAGE_TANGENT, 0,
        D3DDECLUSAGE_NORMAL, 0,
        D3DXTANGENT_CALCULATE_NORMALS | D3DXTANGENT_GENERATE_IN_PLACE,
        &adjacency[0], 0.01f, 0.25f, 0.01f, NULL, NULL));

    return clonedMesh;
}

ID3DXMesh* Mesh::GenerateNormals(ID3DXMesh* mesh, IDirect3DDevice9* d3d9Device)
{
    // see if we have texture coordinates
    D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
    DXCall(mesh->GetDeclaration(decl));
    bool foundTexCoord = false;
    for (UINT i = 0; i < MAX_FVF_DECL_SIZE; ++i)
    {
        if (decl[i].Stream == 0xFF)
            break;
        else if(decl[i].Usage == D3DDECLUSAGE_TEXCOORD && decl[i].UsageIndex == 0)
        {
            foundTexCoord = true;
            break;
        }
    }

    // Clone the mesh with a new declaration
    D3DVERTEXELEMENT9 tcDecl[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    D3DVERTEXELEMENT9 noTCDecl[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    D3DVERTEXELEMENT9* newDecl;
    if (foundTexCoord)
        newDecl = tcDecl;
    else
        newDecl = noTCDecl;

    ID3DXMesh* clonedMesh = NULL;

    UINT options = D3DXMESH_MANAGED;
    if (indexType == Index32Bit)
        options |= D3DXMESH_32BIT;
    DXCall(mesh->CloneMesh(options, newDecl, d3d9Device, &clonedMesh));
    mesh->Release();

    // Generate the normals
    DXCall(D3DXComputeNormals(clonedMesh, &adjacency[0]));

    return clonedMesh;
}

void Mesh::CreateInputElements(D3DVERTEXELEMENT9* declaration)
{
    map<BYTE, LPCSTR> nameMap;
    nameMap[D3DDECLUSAGE_POSITION] = "POSITION";
    nameMap[D3DDECLUSAGE_BLENDWEIGHT] = "BLENDWEIGHT";
    nameMap[D3DDECLUSAGE_BLENDINDICES] = "BLENDINDICES";
    nameMap[D3DDECLUSAGE_NORMAL] = "NORMAL";
    nameMap[D3DDECLUSAGE_TEXCOORD] = "TEXCOORD";
    nameMap[D3DDECLUSAGE_TANGENT] = "TANGENT";
    nameMap[D3DDECLUSAGE_BINORMAL] = "BINORMAL";
    nameMap[D3DDECLUSAGE_COLOR] = "COLOR";

    map<BYTE, DXGI_FORMAT> formatMap;
    formatMap[D3DDECLTYPE_FLOAT1] = DXGI_FORMAT_R32_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT2] = DXGI_FORMAT_R32G32_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT3] = DXGI_FORMAT_R32G32B32_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT4] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    formatMap[D3DDECLTYPE_D3DCOLOR] = DXGI_FORMAT_R8G8B8A8_UNORM;
    formatMap[D3DDECLTYPE_UBYTE4] = DXGI_FORMAT_R8G8B8A8_UINT;
    formatMap[D3DDECLTYPE_SHORT2] = DXGI_FORMAT_R16G16_SINT;
    formatMap[D3DDECLTYPE_SHORT4] = DXGI_FORMAT_R16G16B16A16_SINT;
    formatMap[D3DDECLTYPE_UBYTE4N] = DXGI_FORMAT_R8G8B8A8_UNORM;
    formatMap[D3DDECLTYPE_SHORT2N] = DXGI_FORMAT_R16G16_SNORM;
    formatMap[D3DDECLTYPE_SHORT4N] = DXGI_FORMAT_R16G16B16A16_SNORM;
    formatMap[D3DDECLTYPE_USHORT2N] = DXGI_FORMAT_R16G16_UNORM;
    formatMap[D3DDECLTYPE_USHORT4N] = DXGI_FORMAT_R16G16B16A16_UNORM;
    formatMap[D3DDECLTYPE_UDEC3] = DXGI_FORMAT_R10G10B10A2_UINT;
    formatMap[D3DDECLTYPE_DEC3N] = DXGI_FORMAT_R10G10B10A2_UNORM;
    formatMap[D3DDECLTYPE_FLOAT16_2] = DXGI_FORMAT_R16G16_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT16_4] = DXGI_FORMAT_R16G16B16A16_FLOAT;

    // Figure out the number of elements
    UINT numInputElements = 0;
    while(declaration[numInputElements].Stream != 0xFF)
    {
        D3DVERTEXELEMENT9& element9 = declaration[numInputElements];
        D3D11_INPUT_ELEMENT_DESC element11;
        element11.InputSlot = 0;
        element11.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element11.InstanceDataStepRate = 0;
        element11.SemanticName = nameMap[element9.Usage];
        element11.Format = formatMap[element9.Type];
        element11.AlignedByteOffset = element9.Offset;
        element11.SemanticIndex = element9.UsageIndex;
        inputElements.push_back(element11);

        numInputElements++;
    }
}

// Does a basic draw of all parts
void Mesh::Render(ID3D11DeviceContext* context)
{
    // Set the vertices and indices
    ID3D11Buffer* vertexBuffers[1] = { vertexBuffer };
    UINT vertexStrides[1] = { vertexStride };
    UINT offsets[1] = { 0 };
    context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, offsets);
    context->IASetIndexBuffer(indexBuffer, IndexBufferFormat(), 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw each MeshPart
    for(size_t i = 0; i < meshParts.size(); ++i)
    {
        MeshPart& meshPart = meshParts[i];
        context->DrawIndexed(meshPart.IndexCount, meshPart.IndexStart, 0);
    }
}

// == Model =======================================================================================

Model::Model()
{
}

Model::~Model()
{
    for(size_t i = 0; i < meshMaterials.size(); ++i)
    {
        if(meshMaterials[i].DiffuseMap)
            meshMaterials[i].DiffuseMap->Release();
        if(meshMaterials[i].NormalMap)
            meshMaterials[i].NormalMap->Release();
    }
}

void Model::CreateFromXFile(ID3D11Device* device, LPCWSTR fileName,
    const WCHAR* normalMapSuffix, bool generateNormals,
    bool generateTangentFrame, Mesh::IndexType idxType)
{
    _ASSERT(FileExists(fileName));

    IDirect3DDevice9Ptr d3d9Device = CreateD3D9Device();

    // Load the D3DX mesh
    ID3DXMesh* d3dxMesh = NULL;
    ID3DXBuffer* adjacencyBuffer = NULL;
    ID3DXBuffer* materialsBuffer = NULL;
    DWORD numMaterials = 0;

    UINT options = D3DXMESH_MANAGED;
    if (idxType == Mesh::Index32Bit)
        options |= D3DXMESH_32BIT;
    DXCall(D3DXLoadMeshFromXW(fileName, options, d3d9Device, &adjacencyBuffer,
        &materialsBuffer, NULL, &numMaterials, &d3dxMesh));

    IUnknownReleaser<ID3DXMesh> meshReleaser(d3dxMesh);
    IUnknownReleaser<ID3DXBuffer> adjReleaser(adjacencyBuffer);
    IUnknownReleaser<ID3DXBuffer> matReleaser(materialsBuffer);

    DWORD* initalAdjacency = reinterpret_cast<DWORD*>(adjacencyBuffer->GetBufferPointer());
    D3DXMATERIAL* materials = reinterpret_cast<D3DXMATERIAL*>(materialsBuffer->GetBufferPointer());

    // Get the directory the mesh was loaded from
    wstring fileDirectory = GetDirectoryFromFileName(fileName);

    // Convert materials
    for (UINT i = 0; i < numMaterials; ++i)
    {
        MeshMaterial material;
        D3DXMATERIAL& srcMaterial = materials[i];

        material.AmbientAlbedo = XMFLOAT3(reinterpret_cast<float*>(&srcMaterial.MatD3D.Ambient));
        material.DiffuseAlbedo = XMFLOAT3(reinterpret_cast<float*>(&srcMaterial.MatD3D.Diffuse));
        material.SpecularAlbedo = XMFLOAT3(reinterpret_cast<float*>(&srcMaterial.MatD3D.Specular));
        material.Emissive = XMFLOAT3(reinterpret_cast<float*>(&srcMaterial.MatD3D.Emissive));
        material.SpecularPower = srcMaterial.MatD3D.Power;
        material.Alpha = srcMaterial.MatD3D.Diffuse.a;
        material.DiffuseMapName = AnsiToWString(srcMaterial.pTextureFilename);

        // Add the normal map prefix
        if (normalMapSuffix && material.DiffuseMapName.length() > 0)
        {
            wstring base = GetFileNameWithoutExtension(material.DiffuseMapName.c_str());
            wstring extension = GetFileExtension(material.DiffuseMapName.c_str());
            material.NormalMapName = base + normalMapSuffix + L"." + extension;
        }

        LoadMaterialResources(material, fileDirectory, device);
    }

    // Make a single mesh
    Mesh mesh;
    mesh.CreateFromD3DXMesh(fileDirectory, device, d3d9Device, d3dxMesh, generateNormals, generateTangentFrame, initalAdjacency, idxType);
    meshes.push_back(mesh);
}

void Model::CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName)
{
    _ASSERT(FileExists(fileName));

    // Use the SDKMesh class to load in the data
    SDKMesh sdkMesh;
    sdkMesh.Create(fileName);

    wstring directory = GetDirectoryFromFileName(fileName);

    // Make materials
    UINT numMaterials = sdkMesh.GetNumMaterials();
    for (UINT i = 0; i < numMaterials; ++i)
    {
        MeshMaterial material;
        SDKMESH_MATERIAL* mat = sdkMesh.GetMaterial(i);
        memcpy(&material.AmbientAlbedo, &mat->Ambient, sizeof(D3DXVECTOR4));
        memcpy(&material.DiffuseAlbedo, &mat->Diffuse, sizeof(D3DXVECTOR4));
        memcpy(&material.SpecularAlbedo, &mat->Specular, sizeof(D3DXVECTOR4));
        memcpy(&material.Emissive, &mat->Emissive, sizeof(D3DXVECTOR4));
        material.Alpha = mat->Diffuse.w;
        material.SpecularPower = mat->Power;
        material.DiffuseMapName = AnsiToWString(mat->DiffuseTexture);
        material.NormalMapName = AnsiToWString(mat->NormalTexture);

        LoadMaterialResources(material, directory, device);

        meshMaterials.push_back(material);
    }

    // Make a D3D9 device
    IDirect3DDevice9Ptr d3d9Device = CreateD3D9Device();

    UINT numMeshes = sdkMesh.GetNumMeshes();
    for (UINT meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
    {
        // Figure out the index type
        UINT ops = D3DXMESH_MANAGED;
        UINT indexSize = 2;
        Mesh::IndexType indexType = Mesh::Index16Bit;
        if (sdkMesh.GetIndexType(meshIdx) == IT_32BIT)
        {
            ops |= D3DXMESH_32BIT;
            indexSize = 4;
            indexType = Mesh::Index32Bit;
        }

        // Make a D3DX mesh
        ID3DXMesh* d3dxMesh = NULL;
        UINT numPrims = static_cast<UINT>(sdkMesh.GetNumIndices(meshIdx) / 3);
        UINT numVerts = static_cast<UINT>(sdkMesh.GetNumVertices(meshIdx, 0));
        UINT vbIndex = sdkMesh.GetMesh(meshIdx)->VertexBuffers[0];
        UINT ibIndex = sdkMesh.GetMesh(meshIdx)->IndexBuffer;
        const D3DVERTEXELEMENT9* vbElements = sdkMesh.VBElements(vbIndex);
        DXCall(D3DXCreateMesh(numPrims, numVerts, ops, vbElements, d3d9Device, &d3dxMesh));
        IUnknownReleaser<ID3DXMesh> meshReleaser(d3dxMesh);

        // Copy in vertex data
        BYTE* verts = NULL;
        BYTE* srcVerts = reinterpret_cast<BYTE*>(sdkMesh.GetRawVerticesAt(vbIndex));
        UINT vbStride = sdkMesh.GetVertexStride(meshIdx, 0);
        UINT declStride = D3DXGetDeclVertexSize(vbElements, 0);
        DXCall(d3dxMesh->LockVertexBuffer(0, reinterpret_cast<void**>(&verts)));
        for (UINT vertIdx = 0; vertIdx < numVerts; ++vertIdx)
        {
            memcpy(verts, srcVerts, declStride);
            verts += declStride;
            srcVerts += vbStride;
        }
        DXCall(d3dxMesh->UnlockVertexBuffer());

        // Copy in index data
        void* indices = NULL;
        void* srcIndices = sdkMesh.GetRawIndicesAt(ibIndex);
        DXCall(d3dxMesh->LockIndexBuffer(0, &indices));
        memcpy(indices, srcIndices, numPrims * 3 * indexSize);
        DXCall(d3dxMesh->UnlockIndexBuffer());

        // Set up the attribute table
        DWORD* attributeBuffer = NULL;
        DXCall(d3dxMesh->LockAttributeBuffer(0, &attributeBuffer));

        UINT numSubsets = sdkMesh.GetNumSubsets(meshIdx);
        D3DXATTRIBUTERANGE* attributes = new D3DXATTRIBUTERANGE[numSubsets];
        ArrayDeleter<D3DXATTRIBUTERANGE> attributeDeleter(attributes);
        for (UINT i = 0; i < numSubsets; ++i)
        {
            SDKMESH_SUBSET* subset = sdkMesh.GetSubset(meshIdx, i);
            attributes[i].AttribId = subset->MaterialID;
            attributes[i].FaceStart = static_cast<DWORD>(subset->IndexStart / 3);
            attributes[i].FaceCount = static_cast<DWORD>(subset->IndexCount / 3);
            attributes[i].VertexStart = static_cast<DWORD>(subset->VertexStart);
            // attributes[i].VertexCount = static_cast<DWORD>(subset->VertexCount);
            attributes[i].VertexCount = numVerts;

            for (UINT faceIdx = attributes[i].FaceStart; faceIdx < attributes[i].FaceStart + attributes[i].FaceCount; ++faceIdx)
                attributeBuffer[faceIdx] = subset->MaterialID;
        }

        DXCall(d3dxMesh->UnlockAttributeBuffer());

        d3dxMesh->SetAttributeTable(attributes, numSubsets);

        // Generate initial adjacency
        vector<DWORD> initialAdjacency;
        initialAdjacency.resize(d3dxMesh->GetNumFaces() * 3);
        DXCall(d3dxMesh->GenerateAdjacency(0.0001f, &initialAdjacency[0]));

        // Make the mesh
        Mesh mesh;
        mesh.CreateFromD3DXMesh(directory, device, d3d9Device, d3dxMesh, false, false, &initialAdjacency[0], indexType);
        meshes.push_back(mesh);
    }
}

void Model::LoadMaterialResources(MeshMaterial& material, const wstring& directory, ID3D11Device* device)
{
    // Load the diffuse map
    wstring diffuseMapPath = directory + material.DiffuseMapName;
    if (material.DiffuseMapName.length() > 0 && FileExists(diffuseMapPath.c_str()))
        DXCall(D3DX11CreateShaderResourceViewFromFileW(device, diffuseMapPath.c_str(), NULL, NULL, &material.DiffuseMap, NULL));
    else
        material.DiffuseMap = NULL;

    // Load the normal map
    wstring normalMapPath = directory + material.NormalMapName;
    if (material.NormalMapName.length() > 0 && FileExists(normalMapPath.c_str()))
        DXCall(D3DX11CreateShaderResourceViewFromFileW(device, normalMapPath.c_str(), NULL, NULL, &material.NormalMap, NULL));
    else
        material.NormalMap = NULL;
}

IDirect3DDevice9Ptr Model::CreateD3D9Device()
{
    // Create a D3D9 device (would make it NULL, but PIX doesn't seem to like that)
    IDirect3D9Ptr d3d9;
    d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));

    D3DPRESENT_PARAMETERS pp;
    pp.BackBufferWidth = 1;
    pp.BackBufferHeight = 1;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = GetDesktopWindow();
    pp.Windowed = true;
    pp.Flags = 0;
    pp.FullScreen_RefreshRateInHz = 0;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    pp.EnableAutoDepthStencil = false;

    IDirect3DDevice9Ptr d3d9Device;
    DXCall(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &d3d9Device));

    return d3d9Device;
}

}
