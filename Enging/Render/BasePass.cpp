#include "BasePass.h"
#include <string>

void BasePass::OnResize()
{

    VertexsAndIndexesInput();
    BuildHeaps();
    if (!Init)
    {
        ReloadDescriptors();
    }
    Init = false;
}

void BasePass::LoadTexture()
{
    BuildDescriptorHeaps();
    CreateDescriptors();
}

void BasePass::VertexsAndIndexesInput()
{
    
}


void BasePass::BuildHeaps()
{
    D3D12_RESOURCE_DESC resourceDesc;
    ZeroMemory(&resourceDesc, sizeof(resourceDesc));
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.MipLevels = 1;

    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Width = d3dApp->mClientWidth;
    resourceDesc.Height = d3dApp->mClientHeight;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    resourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    D3D12_CLEAR_VALUE clearVal;
    clearVal.Color[0] = d3dApp->mClearColor[0];
    clearVal.Color[1] = d3dApp->mClearColor[1];
    clearVal.Color[2] = d3dApp->mClearColor[2];
    clearVal.Color[3] = d3dApp->mClearColor[3];

    D3D12_RENDER_TARGET_VIEW_DESC descRTV;
    ZeroMemory(&descRTV, sizeof(descRTV));
    descRTV.Texture2D.MipSlice = 0;
    descRTV.Texture2D.PlaneSlice = 0;
    descRTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    descRTV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    for (int i = 0; i < GBufferRTCount; i++) {
        clearVal.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        d3dApp->mResourceHeap->AddResource("GBuffer" + std::to_string(i),
            resourceDesc,
            nullptr,
            0,
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearVal);

        d3dApp->mResourceHeap->CreateRTVDesc("GBuffer" + std::to_string(i), descRTV);

        d3dApp->mResourceHeap->CreateSRVDesc("GBuffer" + std::to_string(i), descSRV);

    }



    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = d3dApp->mClientWidth;
    depthStencilDesc.Height = d3dApp->mClientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

    // Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
    // the depth buffer.  Therefore, because we need to create two views to the same resource:
    //   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    //   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
    // we need to create the depth buffer resource with a typeless format.  
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = d3dApp->m4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = d3dApp->m4xMsaaState ? (d3dApp->m4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = d3dApp->mDepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    d3dApp->mResourceHeap->AddResource("DepthStencilBuffer",
        depthStencilDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear);


    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = d3dApp->mDepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    d3dApp->mResourceHeap->CreateDSVDesc("DepthStencilBuffer", dsvDesc);

    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    d3dApp->mResourceHeap->CreateSRVDesc("DepthStencilBuffer", descSRV);

    // Transition the resource from its initial state to be used as a depth buffer.
    d3dApp->mCommandList->ResourceBarrier(1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            d3dApp->mResourceHeap->GetResource("DepthStencilBuffer"),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_GENERIC_READ));
}


//after texture 
void BasePass::BuildDescriptorHeaps()
{
    //create SRV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //textures + cubetextures + sky + shadow
    srvHeapDesc.NumDescriptors = d3dApp->mTextures.size() + d3dApp->mCubeTextures.size() + 1 + 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(
        &srvHeapDesc, IID_PPV_ARGS(&mSRVHeap)));
    
    //create RTV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = GBufferRTCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(&mRTVHeap)));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(&mDSVHeap)));

}

void BasePass::CreateDescriptors()
{
    //fill out the heap with descriptors
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSRVHeap->GetCPUDescriptorHandleForHeapStart());

    //load 2D textures
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    //!!!!!!!!!!!maybe mTextures mCbvSrvUavDescriptorSize should be copied in class
    for (auto texture_it = d3dApp->mTextures.begin(); texture_it != d3dApp->mTextures.end(); texture_it++)
    {
        srvDesc.Format = (*texture_it)->Resource->GetDesc().Format;
        srvDesc.Texture2D.MipLevels = (*texture_it)->Resource->GetDesc().MipLevels;
        d3dApp->md3dDevice->CreateShaderResourceView((*texture_it)->Resource.Get(), &srvDesc, hDescriptor);
        // next descriptor
        hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);
    }

    //load cube textures
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

    for (auto texture_it = d3dApp->mCubeTextures.begin(); texture_it != d3dApp->mCubeTextures.end(); texture_it++)
    {
        srvDesc.Format = (*texture_it)->Resource->GetDesc().Format;
        srvDesc.Texture2D.MipLevels = (*texture_it)->Resource->GetDesc().MipLevels;
        d3dApp->md3dDevice->CreateShaderResourceView((*texture_it)->Resource.Get(), &srvDesc, hDescriptor);
        hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);
    }

    mSkyTexHeapIndex = (UINT)d3dApp->mTextures.size();
    mShadowMapIndex = mSkyTexHeapIndex + (UINT)d3dApp->mCubeTextures.size();
    mNullCubeSrvIndex = mShadowMapIndex + 1;

    ReloadDescriptors();
}

void BasePass::ReloadDescriptors()
{

    auto srvCpuStart = mSRVHeap->GetCPUDescriptorHandleForHeapStart();
    auto shadowmapSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mShadowMapIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->md3dDevice->CreateShaderResourceView(d3dApp->mResourceHeap->GetResource("ShadowMap"),
        &d3dApp->mResourceHeap->GetSRVDesc("ShadowMap"),
        shadowmapSrv);
    //load 2D textures

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Texture2D.MipLevels = 1;
    auto nullSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mNullCubeSrvIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->md3dDevice->CreateShaderResourceView(nullptr,
        &srvDesc,
        nullSrv);

    //RTV
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRTVHeap->GetCPUDescriptorHandleForHeapStart());


    for (UINT i = 0; i < GBufferRTCount; i++)
    {
        d3dApp->md3dDevice->CreateRenderTargetView(
            d3dApp->mResourceHeap->GetResource("GBuffer" + std::to_string(i)),
            &d3dApp->mResourceHeap->GetRTVDesc("GBuffer" + std::to_string(i)),
            rtvHeapHandle);
        rtvHeapHandle.Offset(1, d3dApp->mRtvDescriptorSize);
    }

    //DSV
    d3dApp->md3dDevice->CreateDepthStencilView(
        d3dApp->mResourceHeap->GetResource("DepthStencilBuffer"),
        &d3dApp->mResourceHeap->GetDSVDesc("DepthStencilBuffer"),
        mDSVHeap->GetCPUDescriptorHandleForHeapStart());
}

void BasePass::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 100, 2, 0);



    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[6];

    // Perfomance TIP: Order from most frequent to least frequent.
    slotRootParameter[0].InitAsConstantBufferView(0);//cbPerObject : register(b0)
    slotRootParameter[1].InitAsConstantBufferView(1);//cbPass : register(b1)
    slotRootParameter[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);// gCubeMap : register(t0);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);// gShadowMap : register(t1);
    slotRootParameter[5].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);// gTextureMaps[10] : register(t2); 

    auto staticSamplers = d3dApp->GetStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(d3dApp->md3dDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void BasePass::BuildShadersAndInputLayout()
{
    mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\BasePass.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\BasePass.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "MATERIALID",0,DXGI_FORMAT_R32G32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
}

void BasePass::BuildPSO()
{
    CD3DX12_RASTERIZER_DESC opaqueRastDesc(D3D12_DEFAULT);
    opaqueRastDesc.CullMode = D3D12_CULL_MODE_NONE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    opaquePsoDesc.pRootSignature = mRootSignature.Get();
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
        mShaders["standardVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
        mShaders["opaquePS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = opaqueRastDesc;
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    d3dApp->SetPSO(opaquePsoDesc);
    opaquePsoDesc.NumRenderTargets = d3dApp->GbufferRTCount;
    for(int i = 0; i < d3dApp->GbufferRTCount;i++)
    opaquePsoDesc.RTVFormats[i] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    
    ThrowIfFailed(d3dApp->md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

}


void BasePass::Update(const GameTimer& gt)
{
    UpdateMainPassCB(gt);
}

void BasePass::Draw()
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { mSRVHeap.Get() };

    d3dApp->mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    d3dApp->mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    d3dApp->mCommandList->SetPipelineState(mPSOs["opaque"].Get());

    //slotRootParameter[1].InitAsConstantBufferView(1);
    auto passCB = d3dApp->mCurrFrameResource->PassCB->Resource();
    d3dApp->mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

    //slotRootParameter[2].InitAsShaderResourceView(0, 1);
    auto matBuffer = d3dApp->mCurrFrameResource->MaterialBuffer->Resource();
    d3dApp->mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    skyTexDescriptor.Offset(mSkyTexHeapIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

    CD3DX12_GPU_DESCRIPTOR_HANDLE ShadowMapDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    ShadowMapDescriptor.Offset(mShadowMapIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(4, ShadowMapDescriptor);

    //slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(5, mSRVHeap->GetGPUDescriptorHandleForHeapStart());

    d3dApp->mCommandList->RSSetViewports(1, &d3dApp->GetmScreenViewport());
    d3dApp->mCommandList->RSSetScissorRects(1, &d3dApp->GetmScissorRect());

    // Clear the back buffer and depth buffer.
    auto Gbuffer_Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        mRTVHeap->GetCPUDescriptorHandleForHeapStart(),
        0,
        d3dApp->mRtvDescriptorSize);
    auto Gbuffer_Handle_c = Gbuffer_Handle;
    for (int i = 0; i < d3dApp->GbufferRTCount; i++)
    {
        d3dApp->mCommandList->ClearRenderTargetView(Gbuffer_Handle_c, d3dApp->mClearColor, 0, nullptr);
        Gbuffer_Handle_c.Offset(1, d3dApp->mRtvDescriptorSize);
    }

    d3dApp->mCommandList->ResourceBarrier(1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            d3dApp->mResourceHeap->GetResource("DepthStencilBuffer"),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            D3D12_RESOURCE_STATE_DEPTH_WRITE));

    d3dApp->mCommandList->ClearDepthStencilView(mDSVHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,
        0,
        0,
        nullptr);

    // Specify the buffers we are going to render to.

    d3dApp->mCommandList->OMSetRenderTargets(d3dApp->GbufferRTCount, &Gbuffer_Handle, true, &mDSVHeap->GetCPUDescriptorHandleForHeapStart());

    d3dApp->DrawRenderItems(d3dApp->mCommandList, d3dApp->mRitemLayer[(int)RenderLayer::Sky]);
    d3dApp->DrawRenderItems(d3dApp->mCommandList, d3dApp->mRitemLayer[(int)RenderLayer::Opaque]);

}

void BasePass::UpdateMainPassCB(const GameTimer& gt)
{
    XMMATRIX view = d3dApp->mCamera.GetView();
    XMMATRIX proj = d3dApp->mCamera.GetProj();

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
 
    XMMATRIX shadowTransform = XMLoadFloat4x4(&d3dApp->mShadowTransform);

    XMStoreFloat4x4(&mBasePassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mBasePassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mBasePassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mBasePassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mBasePassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mBasePassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&mBasePassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));



    mBasePassCB.EyePosW = d3dApp->mCamera.GetPosition3f();
    mBasePassCB.RenderTargetSize = XMFLOAT2((float)d3dApp->mClientWidth, (float)d3dApp->mClientHeight);
    mBasePassCB.InvRenderTargetSize = XMFLOAT2(1.0f / d3dApp->mClientWidth, 1.0f / d3dApp->mClientHeight);
    mBasePassCB.NearZ = 1.0f;
    mBasePassCB.FarZ = 1000.0f;
    mBasePassCB.TotalTime = gt.TotalTime();
    mBasePassCB.DeltaTime = gt.DeltaTime();
    //should be fixed in lightPass
    mBasePassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
    mBasePassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mBasePassCB.Lights[0].Strength = { 0.9f, 0.8f, 0.7f };
    mBasePassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mBasePassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mBasePassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mBasePassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    auto currPassCB = d3dApp->mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mBasePassCB);
}



