#pragma once
#include "Pass.h"
#include "ShadowMap.h"

class MixPass : public pass
{
public:

	MixPass(D3DINF* d3dapp) : pass(d3dapp) {};

	virtual void Update(const GameTimer& gt) override;
	virtual void Draw() override;
	//virtual void Initialize() override;


	std::unique_ptr<ShadowMap> mShadowMap;

private:
	ComPtr<ID3D12Resource> mViewCB;
	ComPtr<ID3D12Resource> mLightCB;
	ComPtr<ID3D12Resource> mDsTexture;

	UINT mDepthIndex = 0;
	UINT mProfileIndex = 0;
	UINT mLightIndex = 0;

	//std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;//��һ���ر����������RT
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	ComPtr<ID3D12Resource> mVB;
	D3D12_VERTEX_BUFFER_VIEW mVbView;

private:

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps() override;
	virtual void BuildDescriptorHeaps() override;
	virtual void CreateDescriptors() override;
	virtual void BuildRootSignature() override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO() override;
};