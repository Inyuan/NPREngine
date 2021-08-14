#include "Common.hlsl"

Texture2D SceneDepth: register(t0);
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

Texture2D GBuffer[5] : register(t1);


cbuffer cbPass : register(b1)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gShadowTransform;
	float3 gEyePosW;
	float cbPerObjectPad1;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;

	Light gLights[MaxLights];
};

struct vs_in {
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct vs_out {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

vs_out  VS(vs_in vIn)
{
	vs_out vOut;
	vOut.position = vIn.position;
	vOut.texcoord = vIn.texcoord;
	return vIn;
}


float4 PS(vs_out pIn) : SV_TARGET
{
	uint  meterialId = GBuffer[1][pIn.position.xy].a;
	MaterialData matData = gMaterialData[meterialId];
	float thick = matData.thick;
	float Depthstrength = matData.Depthstrength;
	float Depthdistance = matData.Depthdistance;
	float Normalstrength = matData.Normalstrength;
	float Normaldistance = matData.Normaldistance;

	float4 OutColor = float4(1.0f, 1.0f, 1.0f, 1.0f); //空
	//laplace
	float2 uvu = float2(0.0f, -1.0f); //下面坐标
	float2 uvn = float2(0.0f, 1.0f); //上面坐标
	float2 uvl = float2(-1.0f, 0.0f); //左边坐标
	float2 uvr = float2(1.0f, 0.0f); //右边坐标

	float3 bc = GBuffer[0][pIn.position.xy].rgb;
	float3 bu = GBuffer[0][pIn.position.xy + uvu].rgb;
	float3 bn = GBuffer[0][pIn.position.xy + uvn].rgb;
	float3 bl = GBuffer[0][pIn.position.xy + uvl].rgb;
	float3 br = GBuffer[0][pIn.position.xy + uvr].rgb;

	//
	float3 bcSbu = -bc + bu;
	float3 bcSbn = -bc + bn;
	float3 bcSbl = -bc + bl;
	float3 bcSbr = -bc + br;
	float3 B = bcSbu + bcSbn + bcSbl + bcSbr;
	float ba = max(max(B.r, B.g), B.b);
	float bb = pow(ba * Depthstrength, Depthdistance);


	//取深度
	float dc = SceneDepth[pIn.position.xy].r;
	float du = SceneDepth[pIn.position.xy + uvu].r;
	float dn = SceneDepth[pIn.position.xy + uvn].r;
	float dl = SceneDepth[pIn.position.xy + uvl].r;
	float dr = SceneDepth[pIn.position.xy + uvr].r;

	//深度值减减减
	float dcSdu = -dc + du;
	float dcSdn = -dc + dn;
	float dcSdl = -dc + dl;
	float dcSdr = -dc + dr;
	float D = pow((dcSdu + dcSdn + dcSdr + dcSdl) * Normalstrength, Normaldistance); //

	//取法线值
	float3 nc = GBuffer[1][pIn.position.xy].rgb;
	float3 nu = GBuffer[1][pIn.position.xy + uvu].rgb;
	float3 nn = GBuffer[1][pIn.position.xy + uvn].rgb;
	float3 nl = GBuffer[1][pIn.position.xy + uvl].rgb;
	float3 nr = GBuffer[1][pIn.position.xy + uvr].rgb;

	//法线值减减减
	float3 ncSnu = -nc + nu;
	float3 ncSnn = -nc + nn;
	float3 ncSnl = -nc + nl;
	float3 ncSnr = -nc + nr;
	float3 N = ncSnu + ncSnn + ncSnl + ncSnr;
	float na = max(max(N.r,N.g),N.b);
	float nb = pow(na * Normalstrength, Normaldistance);

	//取meterialId
	float mc = GBuffer[1][pIn.position.xy].a;
	float mu = GBuffer[1][pIn.position.xy + uvu].a;
	float mn = GBuffer[1][pIn.position.xy + uvn].a;
	float ml = GBuffer[1][pIn.position.xy + uvl].a;
	float mr = GBuffer[1][pIn.position.xy + uvr].a;
	float mcSmu = -mc + mu;
	float mcSmn = -mc + mn;
	float mcSml = -mc + ml;
	float mcSmr = -mc + mr;
	float M = pow((mcSmu + mcSmn + mcSmr + mcSml) * 10.0f, 0.5f);
	//谁最大
	float a = saturate(max(max(max(nb, D), M), bb));
	
	/*//sobel
	float2 position = pIn.position.xy;
	float2 position_1 = float2(-1.0f,1.0f);
	float2 position_2 = float2(0.0f, 1.0f);
	float2 position_3 = float2(1.0f, 1.0f);
	float2 position_4 = float2(-1.0f, 0.0f);
	float2 position_5 = float2(0.0f, 0.0f);
	float2 position_6 = float2(1.0f, .0f);
	float2 position_7 = float2(-1.0f, -1.0f);
	float2 position_8 = float2(0.0f, -1.0f);
	float2 position_9 = float2(1.0f, -1.0f);

	float da = SceneDepth[position + position_1].r;
	float db = SceneDepth[position + position_2].r;
	float dc = SceneDepth[position + position_3].r;
	float dd = SceneDepth[position + position_4].r;
	float de = SceneDepth[position + position_5].r;
	float df = SceneDepth[position + position_6].r;
	float dg = SceneDepth[position + position_7].r;
	float dh = SceneDepth[position + position_8].r;
	float di = SceneDepth[position + position_9].r;

	float D = pow((dc - dg - da + di + 2 * (df - dd)) + (dc - dg + da - di + 2 * (db - dh)), 0.5);
	D = pow(D * Depthstrength, Depthdistance);

	float3 na = GBuffer[1][position + position_1].rgb;
	float3 nb = GBuffer[1][position + position_2].rgb;
	float3 nc = GBuffer[1][position + position_3].rgb;
	float3 nd = GBuffer[1][position + position_4].rgb;
	float3 ne = GBuffer[1][position + position_5].rgb;
	float3 nf = GBuffer[1][position + position_6].rgb;
	float3 ng = GBuffer[1][position + position_7].rgb;
	float3 nh = GBuffer[1][position + position_8].rgb;
	float3 ni = GBuffer[1][position + position_9].rgb;

	float3 Na = pow((nc - ng - na + ni + 2 * (nf - nd)) +  (nc - ng + na - ni + 2 * (nb - nh)),0.5);
	float N = max(max(Na.r, Na.g), Na.b);
	N = pow(N * Normalstrength, Normaldistance);
	
	//取meterialId
	float mc = GBuffer[1][position].a;
	float mu = GBuffer[1][position + position_2].a;
	float mn = GBuffer[1][position + position_4].a;
	float ml = GBuffer[1][position + position_6].a;
	float mr = GBuffer[1][position + position_8].a;
	float mcSmu = mc - mu;
	float mcSmn = mc - mn;
	float mcSml = mc - ml;
	float mcSmr = mc - mr;
	float M = pow((mcSmu + mcSmn + mcSmr + mcSml) * 10.0f, 0.5f);
	float a = saturate(max(max(N, D), M));
	*/
	

	

	OutColor.r = lerp(10.0f, 0, a); //画黑线
	OutColor.r = step(1.0, a);
	OutColor.g = GBuffer[1][pIn.position.xy].a;
	OutColor.b = thick;
	return OutColor;
}