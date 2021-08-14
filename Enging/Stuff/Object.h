#pragma once
#include "stuff.h"
#include "../Material/Material.h"
#include "../Auxiliary/Fbx.h"
#include "../Auxiliary/GeometryGenerator.h"
#include "../Render/RenderItem.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

enum STARTAND_GEOMETRY
{
	Box,
	Sphere,
	Cylinder,
	Grid,
	Quad
};

struct Vertex
{
	Vertex() {}
	Vertex(
		const DirectX::XMFLOAT3& p,
		const DirectX::XMFLOAT3& n,
		const DirectX::XMFLOAT3& t,
		const DirectX::XMFLOAT2& uv) :
		Position(p),
		Normal(n),
		TangentU(t),
		TexC(uv) {}
	Vertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v) :
		Position(px, py, pz),
		Normal(nx, ny, nz),
		TangentU(tx, ty, tz),
		TexC(u, v) {}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
	DirectX::XMINT2 MaterialId;
};

class Object :public Stuff
{
public:
	Object(std::string name) :Stuff(name) {};

	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;

	private:
		std::vector<uint16> mIndices16;

	BoundingBox bounds;
	std::vector<Vertex> vertices;
	std::vector<std::int32_t> indices;
	//std::vector<int> TriangleMtlIndex;
	XMVECTOR vMax;
	XMVECTOR vMin;

	std::vector<std::unique_ptr<Material>> mMaterials;
	std::vector<std::unique_ptr<MeshGeometry>> mGeometries;
	std::vector<int> verteices_offset;
	std::vector<int> indeices_offset;
	//std::vector<int> mDiffuseTextures_index;
	//std::vector<int> mNormalTextures_index;
	int materialnums = 0;

public:

	bool LoadFbx(std::wstring Filename);
	void LoadTextures(std::wstring Filename,
		std::vector<std::unique_ptr<Texture>>& mTextures,
		Update_ObjectData* upd_obj,
		ID3D12Device* d3ddevice,
		ID3D12GraphicsCommandList* cmdList);
	void CreateGen(STARTAND_GEOMETRY geo);

	void SetMaterial(std::unordered_map<std::string,
		std::vector<std::unique_ptr<Material>>>&Materials,
		UINT& material_size);
	void SetRenderItems(ID3D12Device* d3ddevice,
		ID3D12GraphicsCommandList* cmdList, 
		std::vector<RenderItem*>& RitemLayer,
		UINT RenderLayer,
		std::vector<std::unique_ptr<RenderItem>>& AllRitems,
		UINT& render_nums,
		Update_ObjectData * upd_obj);

private:
	void SetMesh(ID3D12Device* d3ddevice,
		ID3D12GraphicsCommandList* commandList,
		const UINT& render_nums,
		const int material_index);

	void GetTexturesFile(std::vector<std::wstring>& ofiles,
		std::vector<std::string>& filename,
		std::wstring DirPath);

	std::string GetFbxFile(std::wstring DirPath);
};
