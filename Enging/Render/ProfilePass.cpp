#include "ProfilePass.h"

ProfilePass::ProfilePass(D3DINF* d3dapp)
    : pass(d3dapp)
{
    mprofilepassCS = new ProfilePassCS(d3dapp);
};

void ProfilePass::Initialize()
{
    pass::Initialize();
    mprofilepassCS->Initialize();
}

void ProfilePass::OnResize()
{
    pass::OnResize();

    mprofilepassCS->OnResize();
}

void ProfilePass::Draw()
{

    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->mResourceHeap->GetResource("ProfileMap"),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    ID3D12DescriptorHeap* descriptorHeaps[] = { mSRVHeap.Get() };

    d3dApp->mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    d3dApp->mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    d3dApp->mCommandList->SetPipelineState(mPSOs["Profile"].Get());

    d3dApp->mCommandList->SetGraphicsRootConstantBufferView(0, d3dApp->mResourceHeap->GetResource("mViewCB")->GetGPUVirtualAddress());

    auto passCB = d3dApp->mCurrFrameResource->PassCB->Resource();
    d3dApp->mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

    auto matBuffer = d3dApp->mCurrFrameResource->MaterialBuffer->Resource();
    d3dApp->mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

    CD3DX12_GPU_DESCRIPTOR_HANDLE depthDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    depthDescriptor.Offset(mDepthIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(3, depthDescriptor);

    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(4, mSRVHeap->GetGPUDescriptorHandleForHeapStart());

    d3dApp->mCommandList->ClearRenderTargetView(mRTVHeap->GetCPUDescriptorHandleForHeapStart(), d3dApp->mClearColor, 0, nullptr);
    d3dApp->mCommandList->OMSetRenderTargets(1, &mRTVHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);

    d3dApp->mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    d3dApp->mCommandList->IASetVertexBuffers(0, 1, &mVbView);
    d3dApp->mCommandList->DrawInstanced(4, 1, 0, 0);
    //X4->X1
    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->mResourceHeap->GetResource("ProfileMap"),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->mResourceHeap->GetResource("ProfileMap_CMP"),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST));

    d3dApp->mCommandList->ResolveSubresource(d3dApp->mResourceHeap->GetResource("ProfileMap_CMP"), 0, d3dApp->mResourceHeap->GetResource("ProfileMap"), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
    
    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->mResourceHeap->GetResource("ProfileMap_CMP"),
        D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT));
    
    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->mResourceHeap->GetResource("ProfileMap"),
        D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_PRESENT));

    //CS        
    mprofilepassCS->Draw();
}


void ProfilePass::VertexsAndIndexesInput()
{
    struct ScreenQuadVertex
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT2 texcoord;
    };

    ScreenQuadVertex QuadVerts[] =
    {
        { { -1.0f,1.0f, 0.0f,1.0f },{ 0.0f,0.0f } },
        { { 1.0f, 1.0f, 0.0f,1.0f }, {1.0f,0.0f } },
        { { -1.0f, -1.0f, 0.0f,1.0f },{ 0.0f,1.0f } },
    { { 1.0f, -1.0f, 0.0f,1.0f },{ 1.0f,1.0f } }
    };
    //mVB was created in lightPass
    mVbView.BufferLocation = d3dApp->mResourceHeap->GetResource("mVB")->GetGPUVirtualAddress();
    mVbView.StrideInBytes = sizeof(ScreenQuadVertex);
    mVbView.SizeInBytes = sizeof(QuadVerts);
}

void ProfilePass::BuildDescriptorHeaps()
{
    //SRV
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Gbuffer + DepthSentil
    srvHeapDesc.NumDescriptors = d3dApp->GbufferRTCount + 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSRVHeap)));

    //RTV
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(mRTVHeap.GetAddressOf())));


}
void ProfilePass::CreateDescriptors()
{
    //SRV
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSRVHeap->GetCPUDescriptorHandleForHeapStart());


    for (int i = 0; i < d3dApp->GbufferRTCount; i++) {
        d3dApp->md3dDevice->CreateShaderResourceView(
            d3dApp->mResourceHeap->GetResource("GBuffer" + std::to_string(i)),
            &d3dApp->mResourceHeap->GetSRVDesc("GBuffer" + std::to_string(i)),
            hDescriptor); 
        hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);
    }
    mDepthIndex = d3dApp->GbufferRTCount;

    d3dApp->md3dDevice->CreateShaderResourceView(
        d3dApp->mResourceHeap->GetResource("DepthStencilBuffer"),
        &d3dApp->mResourceHeap->GetSRVDesc("DepthStencilBuffer"),
        hDescriptor);

    //RTV
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRTVHeap->GetCPUDescriptorHandleForHeapStart());

    d3dApp->md3dDevice->CreateRenderTargetView(
        d3dApp->mResourceHeap->GetResource("ProfileMap"),
        &d3dApp->mResourceHeap->GetRTVDesc("ProfileMap"),
        rtvHeapHandle);
}

void ProfilePass::BuildHeaps()
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

    D3D12_CLEAR_VALUE clearVal;
    clearVal.Color[0] = d3dApp->mClearColor[0];
    clearVal.Color[1] = d3dApp->mClearColor[1];
    clearVal.Color[2] = d3dApp->mClearColor[2];
    clearVal.Color[3] = d3dApp->mClearColor[3];

    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearVal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    d3dApp->mResourceHeap->AddResource("ProfileMap_CMP",
        resourceDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_PRESENT,
        &clearVal);

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dApp->mResourceHeap->CreateSRVDesc("ProfileMap_CMP", descSRV);


    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.SampleDesc.Count = 8;//MSAA
    resourceDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    resourceDesc.MipLevels = 1;

    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Width = d3dApp->mClientWidth;
    resourceDesc.Height = d3dApp->mClientHeight;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ;

    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearVal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    d3dApp->mResourceHeap->AddResource("ProfileMap",
        resourceDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_PRESENT,
        &clearVal);

    D3D12_RENDER_TARGET_VIEW_DESC descRTV;
    ZeroMemory(&descRTV, sizeof(descRTV));
    descRTV.Texture2D.MipSlice = 0;
    descRTV.Texture2D.PlaneSlice = 0;
    descRTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
    descRTV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dApp->mResourceHeap->CreateRTVDesc("ProfileMap", descRTV);




}

void ProfilePass::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE range[2];
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, d3dApp->GbufferRTCount, 1);

    CD3DX12_ROOT_PARAMETER rootParameters[5];
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsConstantBufferView(1);
    rootParameters[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    rootParameters[3].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(5, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    descRootSignature.NumStaticSamplers = 1;
    descRootSignature.pStaticSamplers = StaticSamplers;


    ComPtr<ID3DBlob> rootSigBlob, errorBlob;

    ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, rootSigBlob.GetAddressOf(), errorBlob.GetAddressOf()));

    ThrowIfFailed(d3dApp->md3dDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf())));

}

void ProfilePass::BuildShadersAndInputLayout()
{
    mShaders["ProfileVS"] = d3dUtil::CompileShader(L"Shaders\\ProfilePass.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["ProfilePS"] = d3dUtil::CompileShader(L"Shaders\\ProfilePass.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void ProfilePass::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
    ZeroMemory(&descPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    descPipelineState.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    descPipelineState.pRootSignature = mRootSignature.Get();
    descPipelineState.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["ProfileVS"]->GetBufferPointer()),
        mShaders["ProfileVS"]->GetBufferSize()
    };
    descPipelineState.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["ProfilePS"]->GetBufferPointer()),
        mShaders["ProfilePS"]->GetBufferSize()
    };
    descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState.DepthEnable = false;
    descPipelineState.SampleMask = UINT_MAX;
    descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    descPipelineState.NumRenderTargets = 1;
    //descPipelineState.RTVFormats[0] = mRtvFormat[0];
    //d3dApp->SetPSO(descPipelineState);
    //descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    //descPipelineState.SampleDesc.Count = 1;

     descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    descPipelineState.SampleDesc.Count = 8;
    descPipelineState.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    descPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ThrowIfFailed(d3dApp->md3dDevice->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&mPSOs["Profile"])));
}


void ProfilePass::Update(const GameTimer& gt)
{
    mprofilepassCS->Update(gt);
}
