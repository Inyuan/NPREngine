#pragma once
#include "Pass.h"

class BasePass : public pass
{
public:

	BasePass(D3DINF* d3dapp) : pass(d3dapp) {};

	//void UpdateObjCB(ObjectConstants objConstants);//BasePass �����ӽ�

	virtual void Update(const GameTimer& gt) override;
	virtual void Draw() override;
	virtual void OnResize() override;

	void LoadTexture();
	
private:

	PassConstants mBasePassCB;  // index 0 of pass cbuffer.

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	//std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;//���������Object������

	//std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;//��һ���ر����������RT


	UINT mSkyTexHeapIndex = 0;
	UINT mNullCubeSrvIndex = 0;
	UINT mShadowMapIndex = 0;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

	bool Init = true;
private:
	void ReloadDescriptors();

	void UpdateMainPassCB(const GameTimer& gt);
	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps() override;
	virtual void BuildDescriptorHeaps() override;
	virtual void CreateDescriptors() override;
	virtual void BuildRootSignature() override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO() override;
};