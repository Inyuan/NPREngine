//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "../Render/BasePass.h"
#include "../Render/LightPass.h"
#include "../Render/ShadowPass.h"
#include "../Render/ProfilePass.h"
#include "../Render/MixPass.h"


#define ENGINEDLL_API __declspec(dllexport) 

class D3DApp
{
protected:

    D3DApp(HWND hwnd);
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();

public:

    static D3DApp* GetApp();
    
	//HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;

    bool Get4xMsaaState()const;
    void Set4xMsaaState(bool value);

    ENGINEDLL_API int Run();
 
    virtual ENGINEDLL_API bool Initialize(int width, int height);

    virtual ENGINEDLL_API void OnResize(int width, int height);
    // Convenience overrides for handling mouse input.
    virtual ENGINEDLL_API void OnMouseDown(int x, int y) { }
    virtual ENGINEDLL_API void OnMouseUp(int x, int y) { }
    virtual ENGINEDLL_API void OnMouseMove(int x, int y) { }


protected:

	virtual void Update(const GameTimer& gt)=0;
    virtual void Draw(const GameTimer& gt)=0;

protected:

	//bool InitMainWindow();
	bool InitDirect3D();
	void CreateCommandObjects();
    void CreateSwapChain();
	void FlushCommandQueue();
	void CalculateFrameStats();

    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);



protected:

    static D3DApp* mApp;

    //HINSTANCE mhAppInst = nullptr; // application instance handle
    HWND      mhMainWnd = nullptr; // main window handle
    bool      mAppPaused = false;  // is the application paused?
    bool      mMinimized = false;  // is the application minimized?
    bool      mMaximized = false;  // is the application maximized?
    bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled
    
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;

    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

public:
    D3DINF* md3dinf;

protected:

    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;


	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption = L"d3d App";

    BasePass* basePass;
    LightPass* lightPass;
    ShadowPass* shadowPass;
    ProfilePass* profilePass;
    MixPass* mixPass;

public:
    int mClientWidth = 800;
    int mClientHeight = 600;
};

