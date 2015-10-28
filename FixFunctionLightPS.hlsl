#include "FixFunctionLight.hlsli"

struct PixelIn {
	float3 PosW		:	POSITION;
	float3 NormalW	:	NORMAL;
	float2 Tex		:	TEXCOORD;
};
Texture2D gDiffuseMap :	register(t0);
SamplerState samState : register(s0);


float4 main(PixelIn pin) : SV_TARGET
{
	//Interpolating normal can un-normalize it
	//need to re-normal it
	pin.NormalW = normalize(pin.NormalW);

	float3 toEye = gEyePosW - pin.PosW;
	float distanceToEye = length(toEye);

	//Normalize the toEye vector since it only has directional meaning, the magnititue is using diffuse,ambient,specular light values
	toEye /= distanceToEye;

	float4 texColor = float4(1, 1, 1, 1);
	[flatten]
	if (gUseTexture) {
		texColor = gDiffuseMap.Sample(samState, pin.Tex);
	}
	[flatten]
	if (gAlphaClip) {

	}
	//set light color defualt to texColor, if no lights, then it's texColor
	float4 lightColor = texColor;
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[flatten]
	if (gDirLightCount > 0) {
		[unroll]
		for (uint i = 0; i < gDirLightCount; ++i) {
			float4 A, D, S;
			A = float4(0.0f, 0.0f, 0.0f, 0.0f);
			D = float4(0.0f, 0.0f, 0.0f, 0.0f);
			S = float4(0.0f, 0.0f, 0.0f, 0.0f);
			ComputeDirectionalLight(gMaterial, gDirLight[i], pin.NormalW, toEye, A, D, S);
			ambient += A;
			diffuse += D;
			spec += S;
		}
	}
	[flatten]
	if (gPointLightCount > 0) {
		[unroll]
		for (uint i = 0; i < gPointLightCount; ++i) {
			float4 A, D, S;
			A = float4(0.0f, 0.0f, 0.0f, 0.0f);
			D = float4(0.0f, 0.0f, 0.0f, 0.0f);
			S = float4(0.0f, 0.0f, 0.0f, 0.0f);
			ComputePointLight(gMaterial, gPointLight[i], pin.PosW, pin.NormalW, toEye, A, D, S);
			ambient += A;
			diffuse += D;
			spec += S;
		}
	}
	[flatten]
	if (gSpotLightCount > 0) {
		[unroll]
		for (uint i = 0; i < gSpotLightCount; ++i) {
			float4 A, D, S;
			A = float4(0.0f, 0.0f, 0.0f, 0.0f);
			D = float4(0.0f, 0.0f, 0.0f, 0.0f);
			S = float4(0.0f, 0.0f, 0.0f, 0.0f);
			ComputeSpotLight(gMaterial, gSpotLight[i], pin.PosW, pin.NormalW, toEye, A, D, S);
			ambient += A;
			diffuse += D;
			spec += S;
		}
	}
	lightColor = texColor * (ambient + diffuse) + spec;
	[flatten]
	if (gReflectionEnable) {

	}
	
	[flatten]
	if (gFogEnable) {
		float fogLerp = saturate((distanceToEye - gFogStart) / gFogRange);
		lightColor = lerp(lightColor, gFogColor, fogLerp);
	}
	lightColor.a = gMaterial.Diffuse.a * texColor.a;
	return lightColor;
}

