#pragma once
#include "D3DLINK.h"
#include "../Render/ResourceHeap.h"
#include "../Auxiliary/FrameResource.h"
#include "../Auxiliary/GameTimer.h"
#include "../Render/RenderItem.h"
#include "../Camera/Camera.h"



class D3DINF
{
public:
    D3DINF() = delete;
    D3DINF(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);

    ID3D12Device* md3dDevice;
    ID3D12GraphicsCommandList* mCommandList;

    ResourceHeap* mResourceHeap = nullptr;
    FrameResource* mCurrFrameResource = nullptr;

    static const int GbufferRTCount = GBufferRTCount;
    UINT mRtvDescriptorSize = 0;
    UINT mDsvDescriptorSize = 0;
    UINT mCbvSrvUavDescriptorSize = 0;

    int mClientWidth = 800;
    int mClientHeight = 600;

    D3D12_VIEWPORT mScreenViewport;//need init
    D3D12_RECT mScissorRect;


    int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];


    std::vector<std::unique_ptr<Texture>> mTextures;
    std::vector< std::unique_ptr<Texture>> mCubeTextures;
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::vector<std::unique_ptr<Material>>> mMaterials;
    
    UINT material_size = 0;
    UINT render_nums = 0;

    // Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

    DXGI_FORMAT mRtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };

    XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();//should be fixed in better way to transform

    Camera mCamera;

    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

public:
    void OnResize();

    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

    ID3D12Resource* GetCurrentBackBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()const;

    D3D12_VIEWPORT  GetmScreenViewport()const;
    D3D12_RECT  GetmScissorRect()const;

    void SetPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

};