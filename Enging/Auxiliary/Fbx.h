#pragma once
#include <fbxsdk.h>
#include <cstdint>
#include <DirectXMath.h>
#include <vector>
#include "FrameResource.h"
class Fbx
{
private:
	FbxManager* lSdkManager;
	FbxScene* lScene;
	void PrintNode(FbxNode* pNode);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	void PrintAttribute(FbxNodeAttribute* pAttribute);
	void ProcessNode(FbxNode* pNode);
	bool ProcessMesh(FbxNode* pNode);
	bool ProcessCamera();
	bool ProcessLight(FbxNode* pNode);
	bool ProcessSkeleton(FbxNode* pNode);

public:

	FbxMesh* pMesh;
	bool ReadFbx(const char* lFilename);
	void ReadVertex(int ctrlPointIndex, DirectX::XMFLOAT3* pVertex);
	void ReadColor(int ctrlPointIndex, int vertexCounter, DirectX::XMFLOAT4* pColor);
	bool ReadUV(int ctrlPointIndex, int textureUVIndex, int uvLayer, DirectX::XMFLOAT2* pUV);
	void ReadNormal(int ctrlPointIndex, int vertexCounter, DirectX::XMFLOAT3* pNormal);
	void ReadTangent(int ctrlPointIndex, int vertecCounter, DirectX::XMFLOAT3* pTangent);

	void PrintFbx();
	
};