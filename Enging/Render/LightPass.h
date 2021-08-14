#pragma once
#include "Pass.h"
#include "ShadowMap.h"

struct CameraData {
	DirectX::XMFLOAT4X4 MVP;
	DirectX::XMFLOAT4X4 InvPV;
	DirectX::XMFLOAT3 CamPos;
};

struct LightData {
	DirectX::XMFLOAT3 pos;

};


class LightPass : public pass
{
public:

	LightPass(D3DINF* d3dapp) : pass(d3dapp) 
	{
	};

	virtual void Update(const GameTimer& gt) override;
	virtual void Draw() override;
	//virtual void Initialize() override;


	std::unique_ptr<ShadowMap> mShadowMap;

private:

	CameraData camData;

	DirectX::BoundingSphere mSceneBounds;

	ComPtr<ID3D12Resource> mViewCB;
	ComPtr<ID3D12Resource> mLightCB;
	ComPtr<ID3D12Resource> mDsTexture;

	UINT mDepthIndex = 0;

	//std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;//需一个特别的类管理缓存和RT
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	ComPtr<ID3D12Resource> mVB;
	D3D12_VERTEX_BUFFER_VIEW mVbView;

private:
	void CreateCB();
	void UpdateConstantBuffer();

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps() override;
	virtual void BuildDescriptorHeaps() override;
	virtual void CreateDescriptors() override;
	virtual void BuildRootSignature() override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO() override;
};