#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H
using namespace DirectX;


struct LAND_DESC {
	float width;
	float depth;
	unsigned int m;
	unsigned int n;
};
struct GRID_DESC {
	float width;
	float depth;
	unsigned int m;
	unsigned int n;
};

struct CYLINDER_DESC {
	float bottomRadius;
	float topRadius;
	float height;
	unsigned int sliceCount;
	unsigned int stackCount;
};

struct GEOSPHERE_DESC {
	float radius;
	unsigned int numSubdivisions;
};
struct SPHERE_DESC {
	float radius;
	unsigned int sliceCount;
	unsigned int stackCount;
};
struct BOX_DESC {
	float width;
	float height;
	float depth;
	unsigned int numSubdivisions;
};
class GeometryGenerator
{
public:
	void CreateLand(const LAND_DESC&, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount);
	void CreateGrid(const GRID_DESC&, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount);
	void CreateCylinder(const CYLINDER_DESC&, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount);
	void CreateGeosphere(const GEOSPHERE_DESC&, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount);
	void CreateSphere(const SPHERE_DESC&, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount);
	void CreateBox(const BOX_DESC&, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount);
private:
	struct Vertex
	{
		Vertex(){};
		Vertex(const XMFLOAT3& p, const XMFLOAT3 & n, const XMFLOAT3& t, const XMFLOAT2& uv) :Position(p), Normal(n), TangentU(t), TexC(uv){}
		Vertex(float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :Position(px, py, pz), Normal(nx, ny, nz), TangentU(tx, ty, tz), TexC(u, v){}
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};


	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	void ConvertMeshData(const MeshData& mesh, void*& vertices, std::vector<unsigned short>& indices);
	void CreateLand(float width, float depth, unsigned int m, unsigned int n, MeshData& meshData);
	void CreateGrid(float width, float depth, unsigned int m, unsigned int n, MeshData& meshData);
	void CreateCylinder(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int sttackCount, MeshData & meshData);
	void CreateGeosphere(float radius, unsigned int numSubdivisions, MeshData& meshData);
	void CreateSphere(float radius, unsigned int sliceCount, unsigned int stackCount, MeshData& meshData);
	void CreateBox(float width, float height, float depth, UINT numSubdivisions, MeshData& meshData);

	void Subdivide(MeshData& meshData);
	void BuildCylinderTopCap(float topRadius, float height, unsigned int sliceCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float height, unsigned int sliceCount, MeshData& meshData);
	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
};
#endif



