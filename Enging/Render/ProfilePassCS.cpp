#include "ProfilePassCS.h"


void ProfilePassCS::Initialize()
{
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildPSO();
}

void ProfilePassCS::Draw()
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { mSRVHeap.Get() };

    d3dApp->mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    d3dApp->mCommandList->SetComputeRootSignature(mRootSignature.Get());
    d3dApp->mCommandList->SetPipelineState(mPSOs["ProfileCS"].Get());
    d3dApp->mCommandList->SetComputeRootDescriptorTable(0, mSRVHeap->GetGPUDescriptorHandleForHeapStart());//UAV绑定根描述符表
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE postprofileDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    postprofileDescriptor.Offset(mPostprofileIndex, d3dApp->mCbvSrvUavDescriptorSize);
    d3dApp->mCommandList->SetComputeRootDescriptorTable(1, postprofileDescriptor);//UAV绑定根描述符表
    
    UINT numGroupsX = (UINT)ceilf(d3dApp->mClientWidth / 32.0f);//X方向的线程组数量
    UINT numGroupsY = (UINT)ceilf(d3dApp->mClientHeight / 32.0f);//Y方向的线程组数量
    d3dApp->mCommandList->Dispatch(numGroupsX, numGroupsY, 1);//分派线程组

}


void ProfilePassCS::VertexsAndIndexesInput()
{   

}

void ProfilePassCS::BuildDescriptorHeaps()
{
    //SRV
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 2;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(d3dApp->md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSRVHeap)));

}

void ProfilePassCS::CreateDescriptors()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSRVHeap->GetCPUDescriptorHandleForHeapStart());

    //SRV
    d3dApp->md3dDevice->CreateShaderResourceView(d3dApp->mResourceHeap->GetResource("ProfileMap_CMP"),
        &d3dApp->mResourceHeap->GetSRVDesc("ProfileMap_CMP"),
        hDescriptor);
    hDescriptor.Offset(1, d3dApp->mCbvSrvUavDescriptorSize);
    /*
    //RTV
    d3dApp->md3dDevice->CreateRenderTargetView(
        d3dApp->mResourceHeap->GetResource("ProfileMap"),
        &d3dApp->mResourceHeap->GetRTVDesc("ProfileMap"),
        mRTVHeap->GetCPUDescriptorHandleForHeapStart());
    */
    mPostprofileIndex = 1;

    d3dApp->md3dDevice->CreateUnorderedAccessView(
        d3dApp->mResourceHeap->GetResource("PostProfileMap"),
        nullptr,
        &d3dApp->mResourceHeap->GetUAVDesc("PostProfileMap"),
        hDescriptor);
}

void ProfilePassCS::BuildHeaps()
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
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_CLEAR_VALUE clearVal;
    clearVal.Color[0] = 1.0f;
    clearVal.Color[1] = 1.0f;
    clearVal.Color[2] = 1.0f;
    clearVal.Color[3] = 1.0f;

    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearVal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    d3dApp->mResourceHeap->AddResource("PostProfileMap",
        resourceDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        &clearVal);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dApp->mResourceHeap->CreateUAVDesc("PostProfileMap", uavDesc);

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dApp->mResourceHeap->CreateSRVDesc("PostProfileMap", descSRV);



}

void ProfilePassCS::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE range[2];
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[2];
    rootParameters[0].InitAsDescriptorTable(1, &range[0]);
    rootParameters[1].InitAsDescriptorTable(1, &range[1]);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    descRootSignature.NumStaticSamplers = 1;
    descRootSignature.pStaticSamplers = StaticSamplers;

    ComPtr<ID3DBlob> rootSigBlob, errorBlob;

    ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, rootSigBlob.GetAddressOf(), errorBlob.GetAddressOf()));

    ThrowIfFailed(d3dApp->md3dDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf())));

}

void ProfilePassCS::BuildShadersAndInputLayout()
{

    mShaders["ProfileCS"] = d3dUtil::CompileShader(L"Shaders\\ProfileCS.hlsl", nullptr, "CS", "cs_5_1");

}

void ProfilePassCS::BuildPSO()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC horizontalBlurPSO = {};
    horizontalBlurPSO.pRootSignature = mRootSignature.Get();
    horizontalBlurPSO.CS =
    {
        reinterpret_cast<BYTE*>(mShaders["ProfileCS"]->GetBufferPointer()),
        mShaders["ProfileCS"]->GetBufferSize()
    };
    horizontalBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    ThrowIfFailed(d3dApp->md3dDevice->CreateComputePipelineState(&horizontalBlurPSO, IID_PPV_ARGS(&mPSOs["ProfileCS"])));
}

void ProfilePassCS::Update(const GameTimer& gt)
{

}
