struct VS_INPUT
{
	float4 pos: POSITION;
	float2 texCoord: TEXCOORD;

	//-----------------------
	// for gpu sknning by kj 
	//-----------------------
	float4 weights: WEIGHTS;
	float4 weightsPosX: WEIGHTS_POS_X;
	float4 weightsPosY: WEIGHTS_POS_Y;
	float4 weightsPosZ: WEIGHTS_POS_Z;
	uint4 boneIndices: BONE_INDICES;
	//-----------------------
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 wvpMat;


	//-----------------------
	// for gpu sknning by kj 
	//-----------------------
	float4 bonePos[256];
	float4 boneRot[256];
	//-----------------------
};

float3 rotateVectorByQuat(float4 q, float3 v) {
	float4 qv = float4(
		q.w * v.x + q.y * v.z - q.z * v.y,
		q.w * v.y + q.z * v.x - q.x * v.z,
		q.w * v.z + q.x * v.y - q.y * v.x,
		-q.x * v.x - q.y * v.y - q.z * v.z
	);

	return float3(
		qv.w * -q.x + qv.x * q.w + qv.y * -q.z - qv.z * -q.y,
		qv.w * -q.y + qv.y * q.w + qv.z * -q.x - qv.x * -q.z,
		qv.w * -q.z + qv.z * q.w + qv.x * -q.y - qv.y * -q.x
	);
}

//-----------------------
// for gpu sknning by kj 
//-----------------------
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	// start gpu skinning
	if (input.boneIndices[0] != 255) {
		float4 tempPos;
		tempPos.x = 0.0f;
		tempPos.y = 0.0f;
		tempPos.z = 0.0f;
		tempPos.w = 1.0f;

		// apply weight and rotation
		for (int i = 0; i < 4; i++) {
			float3 weightPos = float3(input.weightsPosX[i], input.weightsPosY[i], input.weightsPosZ[i]);

			float3 rotPos = rotateVectorByQuat(boneRot[input.boneIndices[i]], weightPos);

			tempPos.x += (bonePos[input.boneIndices[i]].x + rotPos.x) * input.weights[i];
			tempPos.y += (bonePos[input.boneIndices[i]].y + rotPos.y) * input.weights[i];
			tempPos.z += (bonePos[input.boneIndices[i]].z + rotPos.z) * input.weights[i];
		}

		float temp = tempPos.y;
		tempPos.y = tempPos.z;
		tempPos.z = temp;

		// set position 
		output.pos = mul(tempPos, wvpMat);
	}
	else {

		// set position
		output.pos = mul(input.pos, wvpMat);
	}

	// set uv texture coordinates
	output.texCoord = input.texCoord;

	return output;
}
//-----------------------


//VS_OUTPUT main(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	output.pos = mul(input.pos, wvpMat);
//	output.texCoord = input.texCoord;
//	return output;
//}
