
struct DirectionalLight {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; //w = SpecPower
	float4 Reflect;
};


cbuffer cbPerFrame : register(b0) {
	float4x4 gViewProj;
	DirectionalLight gDirLight[3];
	PointLight gPointLight[10];
	SpotLight gSpotLight[2];
	float3 gEyePosW;
	float pad;
	float4 gFogColor;
	float gFogStart;
	float gFogRange;
};
cbuffer cbPerObject : register(b1) {
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gTexTransform;
	Material gMaterial;
};
cbuffer cbAnimationBones : register(b2) {
	float4x4 gBoneTransforms[58];
};
cbuffer control	: register(b3) {
	uint gDirLightCount;
	uint gPointLightCount;
	uint gSpotLightCount;
	uint gUseTexture;
	uint gAlphaClip;
	uint gFogEnable;
	uint gReflectionEnable;
};







void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec) {
	//ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//spec =	  float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -L.Direction;
	ambient = mat.Ambient * L.Ambient;

	float diffuseFactor = dot(lightVec, normal);
	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec) {
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec =	  float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = L.Position - pos;
	float d = length(lightVec);

	//if (d > L.Range) {
	//	return;
	//}

	lightVec /= d;
	ambient = mat.Ambient * L.Ambient;

	float diffuseFactor = dot(lightVec, normal);
	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));
	//ambient used to be not effected by distance att, it used to be no ambient here
	ambient *= att;
	diffuse *= att;
	spec	*= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec) {
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec =	  float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = L.Position - pos;
	float d = length(lightVec);

//	if (d > L.Range) {
//		return;
//	}

	lightVec /= d;
	ambient = mat.Ambient * L.Ambient;

	float diffuseFactor = dot(lightVec, normal);
	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));
	//ambient used to not effective by distance ambient *= spot;
	ambient *= att;
	diffuse *= att;
	spec	*= att;
}