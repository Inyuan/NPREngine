#pragma once
#include "../Game/d3dUtil.h"
#include "../Game/UpdateData.h"

struct MaterialData
{
	DirectX::XMFLOAT3 Lightcolor = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 Specularcolor = { 1.0f, 1.0f, 1.0f};
	DirectX::XMFLOAT3 Shadowcolor = { 0.5f, 0.5f, 0.5f };
	DirectX::XMFLOAT3 Profilecolor= { 0.0f, 0.0f, 0.0f };
	float Thick = 0.f;
	float Shadowsmooth = 0.1f;
	float Shadowsoft = 0.1f;
	float Specularsmooth = 0.1f;
	float Specularsoft = 0.1f;
	
	float Depthstrength = 0.5f;
	float Depthdistance = 3.0f;
	float Normalstrength = 10.0f;
	float Normaldistance = 5.0f;

	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

	UINT DiffuseMapIndex = 0;
	UINT NormalMapIndex = 0;
	UINT LayerIndex = 0;
};

struct Texture
{
	// Unique material name for lookup.
	std::string Name;

	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
	// Unique material name for lookup.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	//int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	//int NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Material constant buffer data used for shading.
	/*DirectX::XMFLOAT3 Lightcolor = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 Specularcolor = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 Shadowcolor = { 0.5f, 0.5f, 0.5f };
	DirectX::XMFLOAT3 Profilecolor = { 0.0f, 0.0f, 0.0f };
	float Thick = 10.f;
	float Shadowsmooth = 0.1f;
	float Shadowsoft = 0.1f;
	float Specularsmooth = 0.1f;
	float Specularsoft = 0.1f;

	float Depthstrength = 0.5f;
	float Depthdistance = 3.0f;
	float Normalstrength = 10.0f;
	float Normaldistance = 5.0f;*/
	Update_MaterialData * upd_Mat;

	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
	UINT LayerIndex = 0;
};