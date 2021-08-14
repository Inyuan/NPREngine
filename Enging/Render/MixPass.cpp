#include "MixPass.h"

void MixPass::Draw()
{
    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    ID3D12DescriptorHeap* descriptorHeaps[] = { mSRVHeap.Get() };

    d3dApp->mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    d3dApp->mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    d3dApp->mCommandList->SetPipelineState(mPSOs["Mix"].Get());

    d3dApp->mCommandList->SetGraphicsRootConstantBufferView(0, d3dApp->mResourceHeap->GetResource("mViewCB")->GetGPUVirtualAddress());

    auto passCB = d3dApp->mCurrFrameResource->PassCB->Resource();
    d3dApp->mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

    auto matBuffer = d3dApp->mCurrFrameResource->MaterialBuffer->Resource();
    d3dApp->mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

    CD3DX12_GPU_DESCRIPTOR_HANDLE depthDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    depthDescriptor.Offset(mDepthIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(3, depthDescriptor);

    CD3DX12_GPU_DESCRIPTOR_HANDLE profileDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    profileDescriptor.Offset(mProfileIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(4, profileDescriptor);

    CD3DX12_GPU_DESCRIPTOR_HANDLE LightDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    LightDescriptor.Offset(mLightIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(5, LightDescriptor);

    d3dApp->mCommandList->SetGraphicsRootDescriptorTable(6, mSRVHeap->GetGPUDescriptorHandleForHeapStart());

    d3dApp->mCommandList->ClearRenderTargetView(d3dApp->GetCurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    d3dApp->mCommandList->OMSetRenderTargets(1, &d3dApp->GetCurrentBackBufferView(), true, nullptr);

    d3dApp->mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    d3dApp->mCommandList->IASetVertexBuffers(0, 1, &mVbView);
    d3dApp->mCommandList->DrawInstanced(4, 1, 0, 0);

    d3dApp->mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dApp->GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

}


void MixPass::VertexsAndIndexesInput()
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

void MixPass::BuildDescriptorHeaps()
{
    //create SRV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Gbuffer + DepthSentil + profile + light
    srvHeapDesc.NumDescriptors = d3dApp->GbufferRTCount + 1 + 1 + 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSRVHeap)));
}

void MixPass::CreateDescriptors()
{
    //fill out the heap with descriptors
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSRVHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < d3dApp->GbufferRTCount; i++) {
        d3dApp->md3dDevice->CreateShaderResourceView(
            d3dApp->mResourceHeap->GetResource("GBuffer" + std::to_string(i)),
            &d3dApp->mResourceHeap->GetSRVDesc("GBuffer" + std::to_string(i)),
            hDescriptor); 
        hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);
    }

    mDepthIndex = d3dApp->GbufferRTCount;
    mProfileIndex = mDepthIndex + 1;
    mLightIndex = mProfileIndex + 1;

    d3dApp->md3dDevice->CreateShaderResourceView(
        d3dApp->mResourceHeap->GetResource("DepthStencilBuffer"),
        &d3dApp->mResourceHeap->GetSRVDesc("DepthStencilBuffer"),
        hDescriptor);
    hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);

    d3dApp->md3dDevice->CreateUnorderedAccessView(
        d3dApp->mResourceHeap->GetResource("PostProfileMap"),
        nullptr,
        &d3dApp->mResourceHeap->GetUAVDesc("PostProfileMap"),
        hDescriptor);
    hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);

    d3dApp->md3dDevice->CreateShaderResourceView(
        d3dApp->mResourceHeap->GetResource("LightMap"),
        &d3dApp->mResourceHeap->GetSRVDesc("LightMap"),
        hDescriptor);

}

void MixPass::BuildHeaps()
{

}

void MixPass::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE range[4];
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, d3dApp->GbufferRTCount, 2);
    CD3DX12_ROOT_PARAMETER rootParameters[7];
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsConstantBufferView(1);
    rootParameters[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    rootParameters[3].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[5].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[6].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = d3dApp->GetStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(7,
        rootParameters,
        (UINT)staticSamplers.size(),
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> rootSigBlob, errorBlob;

    ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, rootSigBlob.GetAddressOf(), errorBlob.GetAddressOf()));

    ThrowIfFailed(d3dApp->md3dDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf())));

}

void MixPass::BuildShadersAndInputLayout()
{
    mShaders["MixVS"] = d3dUtil::CompileShader(L"Shaders\\MixPass.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["MixPS"] = d3dUtil::CompileShader(L"Shaders\\MixPass.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void MixPass::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
    ZeroMemory(&descPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    descPipelineState.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    descPipelineState.pRootSignature = mRootSignature.Get();
    descPipelineState.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["MixVS"]->GetBufferPointer()),
        mShaders["MixVS"]->GetBufferSize()
    };
    descPipelineState.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["MixPS"]->GetBufferPointer()),
        mShaders["MixPS"]->GetBufferSize()
    };
    descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState.DepthEnable = false;
    descPipelineState.SampleMask = UINT_MAX;
    descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    descPipelineState.NumRenderTargets = 1;
    //descPipelineState.RTVFormats[0] = mRtvFormat[0];
    descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    descPipelineState.SampleDesc.Count = 1;
    d3dApp->SetPSO(descPipelineState);
    ThrowIfFailed(d3dApp->md3dDevice->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&mPSOs["Mix"])));

}



void MixPass::Update(const GameTimer& gt)
{

}
