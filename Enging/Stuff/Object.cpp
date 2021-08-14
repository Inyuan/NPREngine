#include "Object.h"
using namespace fbxsdk;
/*
void LoadMaterialAttribute(FbxSurfaceMaterial* pSurfaceMaterial)
{
    // Get the name of material
    pSurfaceMaterial->GetName();

    FbxPropertyT<FbxDouble3> Ambients;
    FbxPropertyT<FbxDouble3> Diffuse;
    FbxPropertyT<FbxDouble3> Specular;
    FbxPropertyT<FbxDouble3> Emissive;
    FbxPropertyT<FbxDouble> TransparencyFactor;
    FbxPropertyT<FbxDouble> Shininess;
    FbxPropertyT<FbxDouble> ReflectionFactor;

    // Phong material
    if (pSurfaceMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
    {
        // Ambient Color
        Ambients = ((FbxSurfacePhong*)pSurfaceMaterial)->Ambient;
        // ...

        // Diffuse Color
        Diffuse = ((FbxSurfacePhong*)pSurfaceMaterial)->Diffuse;
        // ...

        // Specular Color
        Specular = ((FbxSurfacePhong*)pSurfaceMaterial)->Specular;
        // ...

        // Emissive Color
        Emissive = ((FbxSurfacePhong*)pSurfaceMaterial)->Emissive;
        // ...

        // Opacity
        TransparencyFactor = ((FbxSurfacePhong*)pSurfaceMaterial)->TransparencyFactor;
        // ...

        // Shininess
        Shininess = ((FbxSurfacePhong*)pSurfaceMaterial)->Shininess;
        // ...

        // Reflectivity
        ReflectionFactor = ((FbxSurfacePhong*)pSurfaceMaterial)->ReflectionFactor;
        // ...
        return;
    }

    // Lambert material
    if (pSurfaceMaterial->GetClassId().Is(fbxsdk::FbxSurfaceLambert::ClassId))
    {

        // Ambient Color
        Ambients = ((fbxsdk::FbxSurfaceLambert*)pSurfaceMaterial)->Ambient;
        // ...

        // Diffuse Color
        Diffuse = ((fbxsdk::FbxSurfaceLambert*)pSurfaceMaterial)->Diffuse;
        // ...

        // Emissive Color
        Emissive = ((fbxsdk::FbxSurfaceLambert*)pSurfaceMaterial)->Emissive;
        // ...

        // Opacity
        TransparencyFactor = ((fbxsdk::FbxSurfaceLambert*)pSurfaceMaterial)->TransparencyFactor;
        // ...
        return;
    }
}
*/


std::wstring StringToWString(const std::string& str) {
    int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wide = new wchar_t[num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
    std::wstring w_str(wide);
    delete[] wide;
    return w_str;
}

std::string WstringToString(std::wstring wstr)
{
    std::string result;
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buffer = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    result.append(buffer);
    delete[] buffer;
    return result;
}

void LoadMaterialTexture(FbxSurfaceMaterial* pSurfaceMaterial)
{

    int textureLayerIndex;
    fbxsdk::FbxProperty pProperty;

    for (textureLayerIndex = 0; textureLayerIndex < FbxLayerElement::sTypeTextureCount; ++textureLayerIndex)
    {
        pProperty = pSurfaceMaterial->FindProperty(FbxLayerElement::sTextureNames[textureLayerIndex]);

            int textureCount = pProperty.GetSrcObjectCount<FbxTexture>();

            for (int j = 0; j < textureCount; ++j)
            {
                FbxFileTexture* pTexture = pProperty.GetSrcObject<FbxFileTexture>(j);
                if (pTexture)
                {
                    FBXSDK_printf("           File Texture: %s\n", pTexture->GetFileName());
                    // Use pTexture to load the attribute of current texture...
                }
            }
    }
}


bool Object::LoadFbx(std::wstring Filename)
{
    Fbx myFbx;
    std::string Fbxpath = GetFbxFile(Filename);
    if (Fbxpath.empty()) return false;
    myFbx.ReadFbx(Fbxpath.c_str());
    
    using GeoElement = FbxGeometryElement;
    using LayerElement = FbxLayerElement;
    const int polygonCount = myFbx.pMesh->GetPolygonCount();
    auto controlPoints = myFbx.pMesh->GetControlPoints();
    const int controlPointCount = myFbx.pMesh->GetControlPointsCount();
    fbxsdk::FbxLayerElementArrayTemplate<int>* pMaterialIndices = &myFbx.pMesh->GetElementMaterial()->GetIndexArray();

    int etc = 0;

    vertices.assign((int)polygonCount * 3, Vertex());
    indices.assign((int)polygonCount * 3, 0);
    //TriangleMtlIndex.assign(polygonCount, 0);
    int vertexID = 0;
    for (int polygon = 0; polygon < polygonCount; polygon++) //each polygon
    {
        const int polyVertCount = myFbx.pMesh->GetPolygonSize(polygon);
        for (int polyVert = 0; polyVert < polyVertCount; polyVert++) //each Vertex
        {
            const int cpIndex = myFbx.pMesh->GetPolygonVertex(polygon, polyVert);
            //vertex
            vertices[vertexID].Position = XMFLOAT3(
                controlPoints[cpIndex].mData[0],
                controlPoints[cpIndex].mData[1],
                controlPoints[cpIndex].mData[2]
            );
            //index
            indices[vertexID] = vertexID;

            
            const int uvElementsCount = myFbx.pMesh->GetElementUVCount();
            for (int uvElement = 0; uvElement < uvElementsCount; uvElement++)
            {
                const FbxGeometryElementUV* geometryElementUV = myFbx.pMesh->GetElementUV(uvElement);
                const auto mapMode = geometryElementUV->GetMappingMode();
                const auto refMode = geometryElementUV->GetReferenceMode();
                int directIndex = -1;
                if (GeoElement::eByControlPoint == mapMode)
                {
                    if (GeoElement::eDirect == refMode) directIndex = cpIndex;
                    else if (GeoElement::eIndexToDirect == refMode)
                        directIndex = geometryElementUV->GetIndexArray().GetAt(cpIndex);
                }
                else if (
                    GeoElement::eByPolygonVertex == mapMode
                    && (GeoElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
                    )
                    directIndex = myFbx.pMesh->GetTextureUVIndex(polygon, polyVert);
                if (directIndex == -1) continue;
                FbxVector2 uv = geometryElementUV->GetDirectArray().GetAt(directIndex);
                //UV
                vertices[vertexID].TexC = XMFLOAT2(
                    static_cast<float>(uv.mData[0]),
                    static_cast<float>(uv.mData[1])
                );
            }

            const int normalElementCount = myFbx.pMesh->GetElementNormalCount();
            for (int normalElement = 0; normalElement < normalElementCount; normalElement++)
            {
                const FbxGeometryElementNormal* geometryElementNormal = myFbx.pMesh->GetElementNormal(normalElement);
                const LayerElement::EMappingMode mapMode = geometryElementNormal->GetMappingMode();
                const LayerElement::EReferenceMode refMode = geometryElementNormal->GetReferenceMode();
                int directIndex = -1;
                if (GeoElement::eByPolygonVertex == mapMode) 
                {
                    if (GeoElement::eDirect == refMode) directIndex = vertexID;
                    else if (GeoElement::eIndexToDirect == refMode)
                        directIndex = geometryElementNormal->GetIndexArray().GetAt(vertexID);
                }
                if (directIndex == -1) continue;
                FbxVector4 norm = geometryElementNormal->GetDirectArray().GetAt(directIndex);
                //NORMAL
                vertices[vertexID].Normal = XMFLOAT3(
                    static_cast<float>(norm.mData[0]),
                    static_cast<float>(norm.mData[1]),
                    static_cast<float>(norm.mData[2])
                );
            }

            XMVECTOR P = XMLoadFloat3(&vertices[vertexID].Position);

            XMVECTOR N = XMLoadFloat3(&vertices[vertexID].Normal);

            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
            {
                XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
                XMStoreFloat3(&vertices[vertexID].TangentU, T);
            }
            else
            {
                up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
                XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
                XMStoreFloat3(&vertices[vertexID].TangentU, T);
            }

            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);

            //TriangleMtlIndex[triangleIndex] = materialIndex;
            vertices[vertexID].MaterialId.x = pMaterialIndices->GetAt(polygon);
            if (vertices[vertexID].MaterialId.x != etc)
            {
                etc = vertices[vertexID].MaterialId.x;
                verteices_offset.push_back(vertexID);
                indeices_offset.push_back(vertexID);
                materialnums++;
            }
            vertexID++;
        }
        //number of material types 
        
    }

    materialnums++;
    verteices_offset.push_back(vertexID);
    indeices_offset.push_back(vertexID);

    XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
    XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

    //mMaterials.assign(materialnums,nullptr);

    //三角形的对应材质ID
    /*fbxsdk::FbxLayerElementArrayTemplate<int>* pMaterialIndices;
    FbxGeometryElement::EMappingMode   materialMappingMode = FbxGeometryElement::eNone;
    
    if (myFbx.pMesh->GetElementMaterial())
    {
        pMaterialIndices = &myFbx.pMesh->GetElementMaterial()->GetIndexArray();
        materialMappingMode = myFbx.pMesh->GetElementMaterial()->GetMappingMode();
        int etc = 0;
        if (pMaterialIndices)
        {
            switch (materialMappingMode)
            {
            case FbxGeometryElement::eByPolygon:
            {
                if (pMaterialIndices->GetCount() == polygonCount)
                {
                    for (int triangleIndex = 0; triangleIndex < polygonCount; ++triangleIndex)
                    {
                        int materialIndex = pMaterialIndices->GetAt(triangleIndex);

                        //TriangleMtlIndex[triangleIndex] = materialIndex;
                        vertices[triangleIndex * 3].MaterialId.x = materialIndex;
                        vertices[triangleIndex * 3 + 1].MaterialId.x = materialIndex;
                        vertices[triangleIndex * 3 + 2].MaterialId.x = materialIndex;
                        
                        if (materialIndex != etc)
                        {
                            etc = materialIndex;
                            verteices_offset[materialnums] = triangleIndex * 3 + 2;
                            materialnums++;
                        }
                    }
                }
            }
            break;

            case FbxGeometryElement::eAllSame:
            {
                int lMaterialIndex = pMaterialIndices->GetAt(0);

                for (int triangleIndex = 0; triangleIndex < polygonCount; ++triangleIndex)
                {
                    int materialIndex = pMaterialIndices->GetAt(triangleIndex);
                    //TriangleMtlIndex[triangleIndex] = materialIndex;
                    vertices[triangleIndex * 3].MaterialId.x = materialIndex;
                    vertices[triangleIndex * 3+1].MaterialId.x = materialIndex;
                    vertices[triangleIndex * 3+2].MaterialId.x = materialIndex;
                    if (materialIndex != etc)
                    {
                        etc = materialIndex;
                        verteices_offset[materialnums] = triangleIndex * 3 + 2;
                        materialnums++;
                    }
                }
            }
            }
        }
    }*/

    /*
    FbxNode* pNode = myFbx.pMesh->GetNode();
    int materialCount = pNode->GetMaterialCount();

    if (materialCount > 0)
    {
        for (int materialIndex = 0; materialIndex < materialCount; materialIndex++)
        {
            FbxSurfaceMaterial* pSurfaceMaterial = pNode->GetMaterial(materialIndex);
            //材质参数
            //LoadMaterialAttribute(pSurfaceMaterial);
            //贴图
            LoadMaterialTexture(pSurfaceMaterial);
        }
    }
    */

    return true;
}

void Object::CreateGen(STARTAND_GEOMETRY geo)
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData geomesh;

    switch (geo)
    {
    case Box:
        geomesh = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
        break;
    case Sphere:
        geomesh = geoGen.CreateSphere(0.5f, 20, 20);
        break;
    case Cylinder:
        geomesh = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
        break;
    case Grid:
        geomesh = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
        break;
    case Quad:
        geomesh = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
        break;
    default:
        geomesh = geoGen.CreateSphere(0.5f, 20, 20);
        break;
    }

    vertices.assign(geomesh.Vertices.size(), Vertex());
    indices.clear();
    for (size_t i = 0; i < geomesh.Vertices.size(); ++i)
    {
        vertices[i].Position = geomesh.Vertices[i].Position;
        vertices[i].Normal = geomesh.Vertices[i].Normal;
        vertices[i].TexC = geomesh.Vertices[i].TexC;
        vertices[i].TangentU = geomesh.Vertices[i].TangentU;
    }
    indices.insert(indices.end(), std::begin(geomesh.GetIndices16()), std::end(geomesh.GetIndices16()));

    materialnums++;
    verteices_offset.push_back(geomesh.Vertices.size());
    indeices_offset.push_back(geomesh.Indices32.size());

}

std::string Object::GetFbxFile(std::wstring DirPath)
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    std::string res;
    std::wstring SearchName = DirPath + L"*.fbx";
    int i = 0;
    hFind = FindFirstFile(SearchName.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        
        res = WstringToString(DirPath + data.cFileName);
        FindClose(hFind);
    }
    return res;
}


void Object::GetTexturesFile(std::vector<std::wstring>& ofiles ,
    std::vector<std::string>& filename,
    std::wstring DirPath)
{
    HANDLE hFind;
    WIN32_FIND_DATA data;

    ofiles.clear();

    std::wstring SearchName = DirPath + L"*.dds";
    int i = 0;
    hFind = FindFirstFile(SearchName.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // add the path and file name together
            std::wstring tex_name(data.cFileName);
            filename.push_back(mName + "_T_" + WstringToString(tex_name));
            //printf("1:%s\n", filename.back());
            ofiles.push_back(DirPath + data.cFileName);
            //printf("2:%ws\n", ofiles.back());

        } while (FindNextFile(hFind, &data));

        FindClose(hFind);
    }
}

void Object::LoadTextures(std::wstring Filename,
    std::vector<std::unique_ptr<Texture>> &mTextures,
    Update_ObjectData * upd_obj,
    ID3D12Device* d3ddevice,
    ID3D12GraphicsCommandList* cmdList)
{

    std::vector<std::wstring> texFilenames;

    GetTexturesFile(texFilenames, upd_obj->mTexs, Filename);

    //mDiffuseTextures_index.assign(max((int)upd_obj->mTexs.size(), materialnums), 0);
    //mNormalTextures_index.assign(max((int)upd_obj->mTexs.size(), materialnums), 0);
    upd_obj->mTexs_srv_offset = mTextures.size();
    
    for (int i = 0; i < (int)upd_obj->mTexs.size(); ++i)
    {
        auto texMap = std::make_unique<Texture>();
        texMap->Name = upd_obj->mTexs[i];
        texMap->Filename = texFilenames[i];
        ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3ddevice,
            cmdList, texMap->Filename.c_str(),
            texMap->Resource, texMap->UploadHeap));

        //mDiffuseTextures_index[i] = mTextures.size();
        mTextures.push_back(std::move(texMap));
    }
}

void Object::SetMesh(
    ID3D12Device* d3ddevice ,
    ID3D12GraphicsCommandList* cmdList,
    const UINT& render_nums,
    const int material_index)
{

        //Mesh
        auto geo = std::make_unique<MeshGeometry>();
        geo->Name = mName + "_Mesh_" + std::to_string(material_index);

        const UINT vbByteSize = (UINT)verteices_offset[material_index] * sizeof(Vertex);

        const UINT ibByteSize = (UINT)indeices_offset[material_index] * sizeof(std::int32_t);

        void * Vertex_buf = malloc(vbByteSize);
        void* Index_buf = malloc(ibByteSize);
        memmove(Vertex_buf, vertices.data(), vbByteSize);
        memmove(Index_buf, indices.data(), ibByteSize);

        ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
        CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), Vertex_buf, vbByteSize);

        ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
        CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), Index_buf, ibByteSize);

        geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(d3ddevice,
            cmdList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

        geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(d3ddevice,
            cmdList, indices.data(), ibByteSize, geo->IndexBufferUploader);

        geo->VertexByteStride = sizeof(Vertex);
        geo->VertexBufferByteSize = vbByteSize;
        geo->IndexFormat = DXGI_FORMAT_R32_UINT;
        geo->IndexBufferByteSize = ibByteSize;

        SubmeshGeometry submesh;
        submesh.IndexCount = (UINT)indeices_offset[material_index];
        submesh.StartIndexLocation = 0;
        submesh.BaseVertexLocation = 0;
        submesh.Bounds = bounds;

        geo->DrawArgs[geo->Name] = submesh;

        mGeometries.push_back(std::move(geo));

            

}

void Object::SetMaterial(std::unordered_map<std::string,
    std::vector<std::unique_ptr<Material>>>& Materials,
    UINT& material_size)
{
    material_size += mMaterials.size();

    Materials[mName] = std::move(mMaterials);
}

//texture->material? 
void Object::SetRenderItems(ID3D12Device* d3ddevice,
    ID3D12GraphicsCommandList* cmdList,
    std::vector<RenderItem*>& RitemLayer,
    UINT RenderLayer,
    std::vector<std::unique_ptr<RenderItem>>& AllRitems,
    UINT& render_nums,
    Update_ObjectData * upd_obj)
{

    upd_obj->mMats.assign(materialnums, Update_MaterialData());
    for (int i = 0; i < materialnums; i++)
    {
        //Mesh
        SetMesh(d3ddevice, cmdList, render_nums, i);

        //Material
        auto FbxMat = std::make_unique<Material>();
        FbxMat->Name = mName + "_Material_" + std::to_string(i);
        FbxMat->MatCBIndex = render_nums; // wrong


        upd_obj->mMats[i].name = StringToWString(FbxMat->Name);
        FbxMat->upd_Mat = &upd_obj->mMats[i];
        //FbxMat->upd_Mat->DiffuseSrvHeapIndex = mDiffuseTextures_index[i];//贴图手动修改
        //FbxMat->upd_Mat->NormalSrvHeapIndex = 0;//贴图手动修改
        FbxMat->upd_Mat->DiffuseSrvHeapIndex = upd_obj->mTexs_srv_offset+i;
        FbxMat->upd_Mat->NormalSrvHeapIndex = upd_obj->mTexs_srv_offset;


        FbxMat->LayerIndex = RenderLayer;
        mMaterials.push_back(std::move(FbxMat));

        auto FbxRitem = std::make_unique<RenderItem>();
        //XMStoreFloat4x4(&FbxRitem->World, M);

        FbxRitem->upd_Obj = upd_obj;




        FbxRitem->TexTransform = MathHelper::Identity4x4();
        FbxRitem->ObjCBIndex = render_nums++;
        FbxRitem->Mat = mMaterials[i].get();
        FbxRitem->Geo = mGeometries[i].get();
        FbxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        FbxRitem->IndexCount = FbxRitem->Geo->DrawArgs[FbxRitem->Geo->Name].IndexCount;
        FbxRitem->StartIndexLocation = FbxRitem->Geo->DrawArgs[FbxRitem->Geo->Name].StartIndexLocation;
        FbxRitem->BaseVertexLocation = FbxRitem->Geo->DrawArgs[FbxRitem->Geo->Name].BaseVertexLocation;

        RitemLayer.push_back(FbxRitem.get());
        AllRitems.push_back(std::move(FbxRitem));
    }
}