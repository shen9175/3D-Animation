#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include "Animation.h"
using namespace DirectX;

//return a transformation Matrix which can transform this bone to be the status of position, scale and orientation under any given instant time t
void BoneAnimation::Interpolate(float time, XMFLOAT4X4& M) const {
	if (time <= Keyframes.front().TimePos) {//not at showtime, keep the initial status(postion,orientation and scale)
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	} else if (time >= Keyframes.back().TimePos) {//after showtime, keep the final status(position, orientation and scale)
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	} else {
		for (auto i = 0; i < Keyframes.size() - 1; ++i) {
			if (time >= Keyframes[i].TimePos && time <= Keyframes[i + 1].TimePos) {
				float lerpPercent = (time - Keyframes[i].TimePos / Keyframes[i + 1].TimePos - Keyframes[i].TimePos);
				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime() const {
	float t = BoneAnimations.front().GetStartTime();
	for (auto bone : BoneAnimations) {
		t = fmin(t, bone.GetStartTime());
	}
	return t;
}
float AnimationClip::GetClipEndTime() const {
	float t = BoneAnimations.front().GetEndTime();
	for (auto bone : BoneAnimations) {
		t = fmax(t, bone.GetEndTime());
	}
	return t;
}

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& bonesTransforms) const {
	for (auto i = 0; i < BoneAnimations.size(); ++i) {
		BoneAnimations[i].Interpolate(t, bonesTransforms[i]);
	}
}

void AnimationData::GetFinalTransfroms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const {
	if (mBoneOffsets.empty()) {
		return;
	}
	auto numBones = mBoneOffsets.size();
	std::vector<XMFLOAT4X4> toParentTransforms(numBones);
	mAnimations.at(clipName).Interpolate(timePos, toParentTransforms);//get all bone's status(position,orientation and scale under time timePos in local coordinates)
	
	//calculate final transform matrix that can transform all bones status to root coordinates
	std::vector<XMFLOAT4X4> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];
	XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[0]);
	XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[0]);
	XMStoreFloat4x4(&finalTransforms[0], XMMatrixMultiply(offset, toRoot));
	for (auto i = 1; i < numBones; ++i) {
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
		int parent = mBoneHierarchyTree[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parent]);
		toRoot = XMMatrixMultiply(toParent, parentToRoot);
		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
		offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}

//vertex blending calculation for all skin vertices is done in GPU with vertex shader