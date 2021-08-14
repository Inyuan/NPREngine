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

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

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

float4 Opaque_catoonlights(vs_out pIn)
{
    //Gbuffer read funtion
    float4 gbuffera = GBuffer[0][pIn.position.xy];
    float4 gbufferb = GBuffer[1][pIn.position.xy];
    float4 gbufferc = GBuffer[2][pIn.position.xy];
    float4 gbufferd = GBuffer[3][pIn.position.xy];
    float sencedepth = SceneDepth[pIn.position.xy].r;

    float3  baseColor = gbuffera.rgb;
    float   alpha = gbuffera.a;
    float3  worldnormal = gbufferb.rgb;
    uint   meterialId = gbufferb.a;
    float3  shadowfactors = gbufferc.rgb;
    float   profilefactor = gbufferc.a;
    float3  worldposition = gbufferd.rgb;

    MaterialData matData = gMaterialData[meterialId];
    float3 LightColor = matData.LightColor;
    float3 SpecularColor = matData.SpecularColor;
    float3 Shadowcolor = matData.Shadowcolor;
    float3 ProfileColor = matData.ProfileColor;
    float thick = matData.thick;
    float Shadowsmooth = matData.Shadowsmooth;
    float Shadowsoft = matData.Shadowsoft;
    float Specularsmooth = matData.Specularsmooth;
    float Specularsoft = matData.Specularsoft;

    float3 L = -gLights[0].Direction;
    float3 N = worldnormal;
    float3 V = normalize(gEyePosW - worldposition);
    float3 H = normalize(V + L); // 3 add, 2 mad, 4 mul, 1 rsqrt
    float NoH = saturate(dot(N, H));
    float NoL = dot(N, L);
    NoL = (NoL + 1) / 2;

    float ramp = smoothstep(Shadowsoft - Shadowsmooth * 0.5,
                            Shadowsoft + Shadowsmooth * 0.5,
                            NoL);

    /*if (NoL > 0.49 && Shadowsoft >= 0.49f)
    {
        float ramp2 = smoothstep(Shadowsoft - Shadowsmooth * 0.5,
                                 Shadowsoft + Shadowsmooth * 0.5,
                                 shadowfactors[0]);
        ramp = min(ramp, ramp2);
    }*/

    float3 Diffuse = lerp(Shadowcolor, LightColor, ramp);

    float spec = pow(NoH, Specularsoft * 128.0) * 1.0f;
    //spec = smoothstep(0.55, 0.55, spec);
    spec = smoothstep(0.5 - Specularsmooth * 0.5,
                      0.5 + Specularsmooth * 0.5,
                      spec);
    float3 Specular = SpecularColor * spec;

    float ramp2 = 1.0f;
    if (Shadowsoft >= 0.51f)
    {
        ramp2 = smoothstep(Shadowsoft - Shadowsmooth * 0.5,
            Shadowsoft + Shadowsmooth * 0.5,
            shadowfactors[0]);
    }
    //float3 res_color = gLights[0].Strength* (Diffuse+Specular);
    float3 res_color =(Diffuse + Specular);
    res_color = lerp(Shadowcolor,res_color,ramp2);
    //float4 Color = float4(baseColor * max(Diffuse, Specular), alpha);
    float4 Color = float4(baseColor * res_color, alpha);

    return Color;
}

float4 Sky_lights(vs_out pIn)
{
    float4 gbuffera = GBuffer[0][pIn.position.xy];
    float3  baseColor = gbuffera.rgb;
    return float4(baseColor, 1.0f);
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye,float3 DiffuseAlbedo)
{
    float Shininess = 1.0f;
    float3 FresnelR0 = float3(0.1f, 0.1f, 0.1f);
    float3 Roughness = 0.3f;
    const float m = Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (DiffuseAlbedo + specAlbedo) * lightStrength;
}

float4 PhysicalRender(vs_out pIn)
{
    float4 gbuffera = GBuffer[0][pIn.position.xy];
    float4 gbufferb = GBuffer[1][pIn.position.xy];
    float4 gbufferd = GBuffer[3][pIn.position.xy];
    float3 DiffuseAlbedo = gbuffera.rgb;
    float3  worldnormal = gbufferb.rgb;
    float3  worldposition = gbufferd.rgb;

    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -gLights[0].Direction;
    float3 toEye = normalize(gEyePosW - worldposition);

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, worldnormal), 0.0f);
    float3 lightStrength = gLights[0].Strength * ndotl;

    return float4(BlinnPhong(lightStrength, lightVec, worldnormal, toEye, DiffuseAlbedo),1.0f);
}


float4 PS(vs_out pIn) : SV_TARGET
{
    uint meterialId = GBuffer[1][pIn.position.xy].w;
    MaterialData matData = gMaterialData[meterialId];
    uint Renderlayer = matData.LayerIndex;
    float4 Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    switch (Renderlayer)
    {
    case RENDERLAYER_SKY:
        //Color = float4(1.0f, 0.5f, 0.0f, 1);
        Color = Sky_lights(pIn);
        break;
    case RENDERLAYER_OPAQUE:
        //Color = float4(0.0f, 1.0f, 0.5f, 1);
        Color = Opaque_catoonlights(pIn);
        //Color = PhysicalRender(pIn);
        break;
    default:
        //Color = float4(1.0f, 1.0f, 0.0f, 1);
        Color = Opaque_catoonlights(pIn);
        break;
    }
    return Color;
}

