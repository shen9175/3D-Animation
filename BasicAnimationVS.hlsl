#include "FixFunctionLight.hlsli"



struct VertexIn {
	float3 PosL			:POSITION;
	float3 NormalL		:NORMAL;
	float2 Tex			:TEXCOORD;
	float4 TangentL		:TANGENT;
	float3 Weights		:WEIGHTS;
	uint4 BoneIndices	:BONEINDICES;
};

struct VertexOut {
	float3 PosW		:	POSITION;
	float3 NormalW	:	NORMAL;
	float2 Tex		:	TEXCOORD;
	float4 PosH		:	SV_POSITION;
	float  Clip		:	SV_ClipDistance0;
};

VertexOut main( VertexIn vin) {
	VertexOut vout;
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	//blend skin vertex
	[unroll]
	for (int i = 0; i < 4; ++i) {
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}
	//coordinates transformation
	vout.PosW = mul(float4(posL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);
	vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);

	//transform texture
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	[flatten]
	if (gClipPlaneEnable) {
		vout.Clip = dot(float4(vout.PosW, 1.0f), gClipPlane);
	}
	return vout;
}