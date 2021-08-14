#pragma once
#include "Pass.h"
#include "ShadowMap.h"
#include "ProfilePassCS.h"

class ProfilePass : public pass
{
public:

	ProfilePass(D3DINF* d3dapp);

	virtual void Update(const GameTimer& gt) override;
	virtual void Draw() override;
	virtual void Initialize() override;
	virtual void OnResize() override;

	std::unique_ptr<ShadowMap> mShadowMap;

private:

	DirectX::BoundingSphere mSceneBounds;

	UINT mDepthIndex = 0;

	//std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;//需一个特别的类管理缓存和RT
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12Resource> mVB;
	D3D12_VERTEX_BUFFER_VIEW mVbView;

	ProfilePassCS *  mprofilepassCS;
private:

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps() override;
	virtual void BuildDescriptorHeaps() override;
	virtual void CreateDescriptors() override;
	virtual void BuildRootSignature() override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO() override;
};