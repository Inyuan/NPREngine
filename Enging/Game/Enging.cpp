#include "EngingApp.h"

#include "../Auxiliary/Fbx.h"
#include "../Stuff/Object.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

Object* mFbxObj;
Object* mSky;
Object* mSphere;
Object* mBox;
const int gNumFrameResources = 3;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

EngingApp::EngingApp(HWND hwnd)
    : D3DApp(hwnd)
{

}

EngingApp::~EngingApp()
{

}

bool EngingApp::Initialize(int width, int height)
{
    if (!D3DApp::Initialize(width, height))
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    md3dinf->mCamera.SetPosition(0.0f, 2.0f, -15.0f);

//////////////////////////////////////////
    updateData = UpdateData::GetInstance();

    BuildFbxGeometry();
    BuildFrameResources();

    basePass->LoadTexture();

//////////////////////////////////////////

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}


void EngingApp::OnResize(int width, int height)
{
    D3DApp::OnResize(width, height);

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void EngingApp::Update(const GameTimer& gt)
{
    //OnKeyboardInput(gt);
    md3dinf->mCamera.UpdateViewMatrix();

    // Cycle through the circular frame resource array. 帧循环
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    md3dinf->mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point. 等待围栏
    if (md3dinf->mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < md3dinf->mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(md3dinf->mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    //AnimateMaterials(gt);
    UpdateObjectCBs(gt);
    UpdateMaterialBuffer(gt);
    shadowPass->Update(gt);
    basePass->Update(gt);
    lightPass->Update(gt);
    profilePass->Update(gt);
    mixPass->Update(gt);
}

void EngingApp::Draw(const GameTimer& gt)
{

    shadowPass->Draw();
    basePass->Draw();
    lightPass->Draw();
    profilePass->Draw();
    mixPass->Draw();

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    ThrowIfFailed(mSwapChain->Present(0, 0));
    md3dinf->mCurrBackBuffer = (md3dinf->mCurrBackBuffer + 1) % SwapChainBufferCount;
    md3dinf->mCurrFrameResource->Fence = ++mCurrentFence;
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
    
}

void EngingApp::BuildFrameResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            2, (UINT)mAllRitems.size(), md3dinf->material_size));
    }
}


void EngingApp::BuildFbxGeometry()
{
    Update_ObjectData* skyobj = new Update_ObjectData();
    skyobj->name = L"sky";
    skyobj->Transform[0] = 0.0f;
    skyobj->Transform[1] = 0.0f;
    skyobj->Transform[2] = 0.0f;
    skyobj->Rotation[0] = 0.0f;
    skyobj->Rotation[1] = 0.0f;
    skyobj->Rotation[2] = 0.0f;
    skyobj->Scale[0] = 1000.0f;
    skyobj->Scale[1] = 1000.0f;
    skyobj->Scale[2] = 1000.0f;

    mSky = new Object("Sky");
    mSky->CreateGen(Sphere);
    mSky->LoadTextures(
        L"FBXResouce\\Sky\\Texture\\",
        md3dinf->mTextures,
        skyobj,
        md3dDevice.Get(),
        mCommandList.Get());
    

    Update_ObjectData* fbxobj = new Update_ObjectData();;
    fbxobj->name = L"fbx";
    fbxobj->Transform[0] = 0.0f;
    fbxobj->Transform[1] = 0.0f;
    fbxobj->Transform[2] = 0.0f;
    fbxobj->Rotation[0] = 3.15f;
    fbxobj->Rotation[1] = 0.0f;
    fbxobj->Rotation[2] = 1.5f;
    fbxobj->Scale[0] = 0.1f;
    fbxobj->Scale[1] = 0.1f;
    fbxobj->Scale[2] = 0.1f;

    mFbxObj = new Object("FBX");
    bool Fbx_valuable = mFbxObj->LoadFbx(L"FBXResouce\\FBXr\\");
    if(Fbx_valuable)
    mFbxObj->LoadTextures(
        L"FBXResouce\\FBXr\\Texture\\",
        md3dinf->mTextures,
        fbxobj,
        md3dDevice.Get(),
        mCommandList.Get());

    Update_ObjectData* sphereobj = new Update_ObjectData();
    sphereobj->name = L"sphere";
    sphereobj->Transform[0] = 3.0f;
    sphereobj->Transform[1] = 2.0f;
    sphereobj->Transform[2] = 0.0f;
    sphereobj->Rotation[0] = 0.0f;
    sphereobj->Rotation[1] = 0.0f;
    sphereobj->Rotation[2] = 0.0f;
    sphereobj->Scale[0] = 1.0f;
    sphereobj->Scale[1] = 1.0f;
    sphereobj->Scale[2] = 1.0f;
    mSphere = new Object("Sphere");
    mSphere->CreateGen(Sphere);
    mSphere->LoadTextures(
        L"FBXResouce\\Sphere\\Texture\\",
        md3dinf->mTextures,
        sphereobj,
        md3dDevice.Get(),
        mCommandList.Get());

    Update_ObjectData* boxobj = new Update_ObjectData();
    boxobj->name = L"box";
    boxobj->Transform[0] = 6.0f;
    boxobj->Transform[1] = 2.0f;
    boxobj->Transform[2] = 0.0f;
    boxobj->Rotation[0] = 0.0f;
    boxobj->Rotation[1] = 0.0f;
    boxobj->Rotation[2] = 0.0f;
    boxobj->Scale[0] = 1.0f;
    boxobj->Scale[1] = 1.0f;
    boxobj->Scale[2] = 1.0f;
    
    mBox = new Object("Box");
    mBox->CreateGen(Box);
    mBox->LoadTextures(
        L"FBXResouce\\Box\\Texture\\",
        md3dinf->mTextures,
        boxobj,
        md3dDevice.Get(),
        mCommandList.Get());




    mSky->SetRenderItems(md3dDevice.Get(),
        mCommandList.Get(),
        md3dinf->mRitemLayer[(int)RenderLayer::Sky],
        (UINT)RenderLayer::Sky,
        mAllRitems,
        md3dinf->render_nums,
        skyobj);
    mSky->SetMaterial(md3dinf->mMaterials, md3dinf->material_size);
    updateData->mObj.push_back(skyobj);

    if (Fbx_valuable)
    mFbxObj->SetRenderItems(md3dDevice.Get(),
        mCommandList.Get(),
        md3dinf->mRitemLayer[(int)RenderLayer::Opaque],
        (UINT)RenderLayer::Opaque,
        mAllRitems,
        md3dinf->render_nums,
        fbxobj);
    if(Fbx_valuable)
    mFbxObj->SetMaterial(md3dinf->mMaterials, md3dinf->material_size);
    updateData->mObj.push_back(fbxobj);
    
    mSphere->SetRenderItems(md3dDevice.Get(),
        mCommandList.Get(),
        md3dinf->mRitemLayer[(int)RenderLayer::Opaque],
        (UINT)RenderLayer::Opaque,
        mAllRitems,
        md3dinf->render_nums,
        sphereobj);
    mSphere->SetMaterial(md3dinf->mMaterials, md3dinf->material_size);
    updateData->mObj.push_back(sphereobj);

    mBox->SetRenderItems(md3dDevice.Get(),
        mCommandList.Get(),
        md3dinf->mRitemLayer[(int)RenderLayer::Opaque],
        (UINT)RenderLayer::Opaque,
        mAllRitems,
        md3dinf->render_nums,
        boxobj);
    mBox->SetMaterial(md3dinf->mMaterials, md3dinf->material_size);
    updateData->mObj.push_back(boxobj);



}




void EngingApp::UpdateObjectCBs(const GameTimer& gt)
{

    auto currObjectCB = md3dinf->mCurrFrameResource->ObjectCB.get();
    for (auto& e : mAllRitems)
    {
        if (e->upd_Obj->MashneedRefresh > 0)
        {
            e->NumFramesDirty = gNumFrameResources;
            e->upd_Obj->MashneedRefresh--;
        }
        // Only update the cbuffer data if the constants have changed.  
        // This needs to be tracked per frame resource.
        if (e->NumFramesDirty > 0)
        {
            XMMATRIX world = XMMatrixTranslation(e->upd_Obj->Transform[0],
                e->upd_Obj->Transform[1],
                e->upd_Obj->Transform[2])
                * XMMatrixRotationY(e->upd_Obj->Rotation[0])
                * XMMatrixRotationZ(e->upd_Obj->Rotation[1])
                * XMMatrixRotationX(e->upd_Obj->Rotation[2])
                * XMMatrixScaling(e->upd_Obj->Scale[0],
                    e->upd_Obj->Scale[1],
                    e->upd_Obj->Scale[2]);

            XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
            objConstants.MaterialIndex = e->Mat->MatCBIndex;

            currObjectCB->CopyData(e->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            e->NumFramesDirty--;
        }
    }
}

void EngingApp::UpdateMaterialBuffer(const GameTimer& gt)
{
    auto currMaterialBuffer = md3dinf->mCurrFrameResource->MaterialBuffer.get();
    for (auto& ee : md3dinf->mMaterials)//each object 
    for(auto& e : ee.second)//each materials
    {
        if (e->upd_Mat->needRefresh)
        {
            e->NumFramesDirty = gNumFrameResources;
            e->upd_Mat->needRefresh = false;
        }
        // Only update the cbuffer data if the constants have changed.  If the cbuffer
        // data changes, it needs to be updated for each FrameResource.
        Material* mat = e.get();
        if (mat->NumFramesDirty > 0)
        {
            XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

            MaterialData matData;

            matData.Lightcolor = { mat->upd_Mat->Lightcolor[0],
                mat->upd_Mat->Lightcolor[1],
                mat->upd_Mat->Lightcolor[2] };
            matData.Specularcolor = { mat->upd_Mat->Specularcolor[0],
                mat->upd_Mat->Specularcolor[1],
                mat->upd_Mat->Specularcolor[2] };
            matData.Shadowcolor = { mat->upd_Mat->Shadowcolor[0],
                mat->upd_Mat->Shadowcolor[1],
                mat->upd_Mat->Shadowcolor[2] };
            matData.Profilecolor = { mat->upd_Mat->Profilecolor[0],
                mat->upd_Mat->Profilecolor[1],
                mat->upd_Mat->Profilecolor[2] };
            matData.Thick = mat->upd_Mat->Thick;
            matData.Shadowsmooth = mat->upd_Mat->Shadowsmooth;
            matData.Shadowsoft = mat->upd_Mat->Shadowsoft;
            matData.Specularsmooth = mat->upd_Mat->Specularsmooth;
            matData.Specularsoft = mat->upd_Mat->Specularsoft;

            matData.Depthstrength = mat->upd_Mat->Depthstrength;
            matData.Depthdistance = mat->upd_Mat->Depthdistance;
            matData.Normalstrength = mat->upd_Mat->Normalstrength;
            matData.Normaldistance = mat->upd_Mat->Normaldistance;

            XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
            matData.DiffuseMapIndex = mat->upd_Mat->DiffuseSrvHeapIndex;//贴图手动修改刷新
            matData.NormalMapIndex = mat->upd_Mat->NormalSrvHeapIndex;
            matData.LayerIndex = mat->LayerIndex;
            currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

            // Next FrameResource need to be updated too.
            mat->NumFramesDirty--;
        }
    }
}

void EngingApp::OnMouseDown(int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    mouseDown = true;
    //SetCapture(mhMainWnd);
}

void EngingApp::OnMouseUp(int x, int y)
{
    //ReleaseCapture();
    mLastMousePos.x = x;
    mLastMousePos.y = y;
    mouseDown = false;
}

void EngingApp::OnMouseMove(int x, int y)
{
      // Make each pixel correspond to a quarter of a degree.
    if (mouseDown = true)
    {
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        md3dinf->mCamera.Pitch(dy);
        md3dinf->mCamera.RotateY(dx);
    }
     mLastMousePos.x = x;
     mLastMousePos.y = y;
}

void EngingApp::OnKeyboardInput(int key)
{
    const float dt = mTimer.DeltaTime();

    if (key == 'W' || key == 'w')
        md3dinf->mCamera.Walk(10.0f * dt * 10.f);

    if (key == 'S' || key == 's')
        md3dinf->mCamera.Walk(-10.0f * dt * 10.f);

    if (key == 'A' || key == 'a')
        md3dinf->mCamera.Strafe(-10.0f * dt * 10.f);

    if (key == 'D' || key == 'd')
        md3dinf->mCamera.Strafe(10.0f * dt * 10.f);
}



