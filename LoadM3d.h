#ifndef LOADM3D_H
#define LOADM3D_H

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

struct M3dMaterial
{
	Material Mat;
	bool AlphaClip;
	std::string EffectTypeName;
	std::string DiffuseMapName;
	std::string NormalMapName;
};

struct Subset
{
	Subset() : 
		Id(-1), 
		VertexStart(0), VertexCount(0),
		FaceStart(0), FaceCount(0)
	{
	}

	UINT Id;
	UINT VertexStart;
	UINT VertexCount;
	UINT FaceStart;
	UINT FaceCount;
};



class M3DLoader
{
public:
	/*
	bool LoadM3d(const std::string& filename, 
		std::vector<PosNormalTex>& vertices,
		std::vector<unsigned int>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats);
	bool LoadM3d(const std::string& filename, 
		std::vector<PosNormalTexTan>& vertices,
		std::vector<unsigned int>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats);*/
	bool LoadM3d(const std::string& filename, 
		void*& vertices,
		std::vector<unsigned int>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats,
		AnimationData& skinInfo, std::string vertextype, size_t& numVertex);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets);
	//void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<PosNormalTex>& vertices);
	//void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<PosNormalTexTan>& vertices);
	//void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<PosNormalTexTanAnimation>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, void*& vertices, std::string vertextype);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<unsigned int>& indices);
	void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};



#endif // LOADM3D_H