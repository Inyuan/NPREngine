#pragma once
#include "d3dApp.h"
#include "../Auxiliary/MathHelper.h"
#include "../Auxiliary/DDSTextureLoader.h"
#include "UpdateData.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class EngingApp : public D3DApp
{
public:
    ENGINEDLL_API EngingApp(HWND hwnd);
    ENGINEDLL_API EngingApp(const EngingApp& rhs) = delete;
    ENGINEDLL_API EngingApp& operator=(const EngingApp& rhs) = delete;
    ENGINEDLL_API ~EngingApp();

    virtual ENGINEDLL_API bool Initialize(int width, int height)override;
    virtual ENGINEDLL_API void OnResize(int width, int height)override;

    virtual ENGINEDLL_API void OnMouseDown(int x, int y)override;
    virtual ENGINEDLL_API void OnMouseUp(int x, int y)override;
    virtual ENGINEDLL_API void OnMouseMove(int x, int y)override;
    ENGINEDLL_API void OnKeyboardInput(int key);

private:
    void BuildFrameResources();

    void BuildFbxGeometry();

    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMaterialBuffer(const GameTimer& gt);

public:

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PSO_DESC;
private:

    std::vector<std::unique_ptr<RenderItem>> mAllRitems;
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;

    int mCurrFrameResourceIndex = 0;
    
private:
    
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

private:

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f * XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 5.0f;

    bool mouseDown = false;
    POINT mLastMousePos;

    UpdateData* updateData;
};
