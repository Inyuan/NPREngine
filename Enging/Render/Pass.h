#pragma once
#include "../Auxiliary/D3DINF.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class pass
{
public:
	pass(D3DINF* d3dapp) : d3dApp(d3dapp) { };
	pass(const pass& rhs) = delete;
	pass& operator=(const pass& rhs) = delete;
	~pass() {};

	virtual void Update(const GameTimer& gt) {};
	virtual void Draw() {};

	virtual void Initialize() 
	{
		BuildRootSignature();
		BuildShadersAndInputLayout();
		BuildPSO();
	};

	virtual void OnResize()
	{
		BuildDescriptorHeaps();
		VertexsAndIndexesInput();
		BuildHeaps();
		CreateDescriptors();
	};

protected:
	D3DINF* d3dApp;

	ComPtr<ID3D12DescriptorHeap> mSRVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mRTVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mDSVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCBVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mUAVHeap = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

private:

	

	virtual void VertexsAndIndexesInput() = 0;
	virtual void BuildHeaps() = 0;
	virtual void BuildDescriptorHeaps() = 0;
	virtual void CreateDescriptors() = 0;
	virtual void BuildRootSignature() = 0;
	virtual void BuildShadersAndInputLayout() = 0;
	virtual void BuildPSO() = 0;

};
