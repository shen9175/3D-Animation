#include "FixFunctionLight.hlsli"


struct VertexIn {
	float3 PosL			:POSITION;
	float3 NormalL		:NORMAL;
	float2 Tex			:TEXCOORD;
	float4 TangentL		:TANGENT;
};

struct VertexOut {
	float3 PosW		:	POSITION;
	float3 NormalW	:	NORMAL;
	float2 Tex		:	TEXCOORD;
	float4 PosH		:	SV_POSITION;
};

VertexOut main( VertexIn vin) {
	VertexOut vout;

	//coordinates transformation
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);

	//transform texture
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	return vout;
}