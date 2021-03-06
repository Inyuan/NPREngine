//***************************************************************************************
// Common.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

#define RENDERLAYER_OPAQUE 0
#define RENDERLAYER_SKY 1

struct MaterialData
{
	float3 LightColor;
	float3 SpecularColor;
	float3 Shadowcolor;
	float3 ProfileColor;

	float thick;
	float Shadowsmooth;
	float Shadowsoft;
	float Specularsmooth;
	float Specularsoft;

	float Depthstrength;
	float Depthdistance;
	float Normalstrength;
	float Normaldistance;

	float4x4 MatTransform;

	uint     DiffuseMapIndex;
	uint     NormalMapIndex;
	uint LayerIndex;
};

//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}



