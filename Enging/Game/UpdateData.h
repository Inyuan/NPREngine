#pragma once
#include <vector>
#include <map>
#include <string>

struct Update_ObjectData;
struct Update_MaterialData;

struct Update_ObjectData
{
	float Transform[3] = { 0,0,0 };
	float Rotation[3] = { 0,0,0 };
	float Scale[3] = { 0,0,0 };
	std::wstring name;
	std::vector<Update_MaterialData> mMats;
	std::vector<std::string> mTexs;
	int mTexs_srv_offset = 0;
	int MashneedRefresh = 0;
};

struct Update_MaterialData
{
	std::wstring name;
	float	Lightcolor[3] = { 1.0f, 1.0f, 1.0f };
	float	Specularcolor[3] = { 100.0f, 100.0f, 100.0f };
	float	Shadowcolor[3] = { 0.5f, 0.5f, 0.5f };
	float	Profilecolor[3] = { 0.0f, 0.0f, 0.0f };
	float	Thick = 0.01f;
	float	Shadowsmooth = 0.001f;
	float	Shadowsoft = 0.5f;
	float	Specularsmooth =  0.02f;
	float	Specularsoft = 1.00f;
	float	Depthstrength = 0.5f;
	float	Depthdistance = 0.2f;
	float	Normalstrength = 1.5f;
	float	Normaldistance = 0.1f;

	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = 0;

	bool needRefresh = false;
};

class UpdateData
{
private:

	static UpdateData* UPDInstance;

	
	UpdateData() {};

public:
	std::vector<Update_ObjectData*> mObj;
	
	static __declspec(dllexport) UpdateData* GetInstance();

	~UpdateData();
};
