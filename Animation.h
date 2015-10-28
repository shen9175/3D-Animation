#ifndef ANIMATION_H
#define ANIMATION_H
using namespace DirectX;

struct Keyframe{
	Keyframe() : TimePos(0.0f), Translation(0.0f, 0.0f, 0.0f), Scale(1.0f, 1.0f, 1.0f), RotationQuat(0.0f, 0.0f, 0.0f, 1.0f) {}
	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct BoneAnimation {
	float GetStartTime() const { return Keyframes.front().TimePos; }
	float GetEndTime()const { return Keyframes.back().TimePos; }
	void Interpolate(float time, XMFLOAT4X4& M) const;//return a transformation Matrix which can transform this bone under given time t, to be the calculated position, scale and orientation
	std::vector<Keyframe> Keyframes;
};

struct AnimationClip {
	float GetClipStartTime() const;
	float GetClipEndTime() const;
	void Interpolate(float t, std::vector<XMFLOAT4X4>& bonesTransforms) const;
	std::vector<BoneAnimation> BoneAnimations; //for all bones of this model
};

class AnimationData {
public:
	size_t BoneCount() const { return mBoneHierarchyTree.size(); }
	float GetClipStartTime(const std::string& clipName) const { return mAnimations.at(clipName).GetClipStartTime(); }
	float GetClipEndTime(const std::string& clipName) const { return mAnimations.at(clipName).GetClipEndTime(); }
	void Set(std::vector<int>& boneHierarchy, std::vector<XMFLOAT4X4>& boneOffsets, std::unordered_map<std::string, AnimationClip>& animations) { mBoneHierarchyTree = boneHierarchy; mBoneOffsets = boneOffsets; mAnimations = animations; }
	void GetFinalTransfroms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const;

private:
	std::vector<int> mBoneHierarchyTree;
	std::vector<XMFLOAT4X4> mBoneOffsets;
	std::unordered_map<std::string, AnimationClip> mAnimations;
};
#endif
