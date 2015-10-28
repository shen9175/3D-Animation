#include <d3d11.h>
#include <DirectXMath.h>
#include <algorithm>
#include <unordered_map>
#include "GeometryGenerator.h"
#include "Camera.h"
#include "TextureMangement.h"
#include "Animation.h"
#include "LoadM3d.h"
#include "ConstantBuffer.h"
#include "ShaderMangement.h"
#include "Object3D.h"
using namespace DirectX;


float AngleFromXY(float x, float y)
{
	float theta = 0.0f;

	// Quadrant I or IV
	if(x >= 0.0f) 
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if(theta < 0.0f)
			theta += 2.0f * XM_PI; // in [0, 2*pi).
	}

	// Quadrant II or III
	else      
		theta = atanf(y/x) + XM_PI; // in [0, 2*pi).

	return theta;
}

XMFLOAT2 Lerp(const XMFLOAT2& a, const XMFLOAT2& b, float t)
{
	return XMFLOAT2(
		(1.0f-t)*a.x + t*b.x,
		(1.0f-t)*a.y + t*b.y);
}

XMFLOAT3 Lerp(const XMFLOAT3& a, const XMFLOAT3& b, float t)
{
	return XMFLOAT3(
		(1.0f-t)*a.x + t*b.x,
		(1.0f-t)*a.y + t*b.y,
		(1.0f-t)*a.z + t*b.z);
}

XMFLOAT4 Lerp(const XMFLOAT4& a, const XMFLOAT4& b, float t)
{
	return XMFLOAT4(
		(1.0f-t)*a.x + t*b.x,
		(1.0f-t)*a.y + t*b.y,
		(1.0f-t)*a.z + t*b.z,
		(1.0f-t)*a.w + t*b.w);
}

void  GeometryGenerator::CreateLand(const LAND_DESC& desc, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount) {
	MeshData mesh;
	CreateLand(desc.width, desc.depth, desc.m, desc.n, mesh);
	VertexCount = mesh.Vertices.size();
	ConvertMeshData(mesh, vertices, indices);
}
void  GeometryGenerator::CreateGrid(const GRID_DESC& desc, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount) {
	MeshData mesh;
	CreateGrid(desc.width, desc.depth, desc.m, desc.n, mesh);
	VertexCount = mesh.Vertices.size();
	ConvertMeshData(mesh, vertices, indices);
}
void  GeometryGenerator::CreateCylinder(const CYLINDER_DESC& desc, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount) {
	MeshData mesh;
	CreateCylinder(desc.bottomRadius, desc.topRadius, desc.height, desc.sliceCount, desc.stackCount, mesh);
	VertexCount = mesh.Vertices.size();
	ConvertMeshData(mesh, vertices, indices);
}
void  GeometryGenerator::CreateGeosphere(const GEOSPHERE_DESC& desc, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount) {
	MeshData mesh;
	CreateGeosphere(desc.radius, desc.numSubdivisions, mesh);
	VertexCount = mesh.Vertices.size();
	ConvertMeshData(mesh, vertices, indices);
}
void  GeometryGenerator::CreateSphere(const SPHERE_DESC& desc, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount) {
	MeshData mesh;
	CreateSphere(desc.radius, desc.sliceCount, desc.stackCount, mesh);
	VertexCount = mesh.Vertices.size();
	ConvertMeshData(mesh, vertices, indices);
}
void  GeometryGenerator::CreateBox(const BOX_DESC& desc, void*& vertices, std::vector<unsigned short>& indices, size_t& VertexCount) {
	MeshData mesh;
	CreateBox(desc.width, desc.height, desc.depth, desc.numSubdivisions, mesh);
	VertexCount = mesh.Vertices.size();
	ConvertMeshData(mesh, vertices, indices);
}

void GeometryGenerator::ConvertMeshData(const MeshData& mesh, void*& vertices, std::vector<unsigned short>& indices) {
	PosNormalTexTan * p = new PosNormalTexTan[mesh.Vertices.size()];
	for (size_t i = 0; i < mesh.Vertices.size(); ++i) {
		p[i].Pos = mesh.Vertices[i].Position;
		p[i].Normal = mesh.Vertices[i].Normal;
		p[i].Tex = mesh.Vertices[i].TexC;
		p[i].TangentU.x = mesh.Vertices[i].TangentU.x;
		p[i].TangentU.y = mesh.Vertices[i].TangentU.y;
		p[i].TangentU.z = mesh.Vertices[i].TangentU.z;
		p[i].TangentU.w = 1;//need for more information later
	}
	vertices = (void*)p;
	for (auto item : mesh.Indices) {
		indices.push_back(static_cast<unsigned short>(item));
	}
}

float GeometryGenerator::GetHillHeight(float x, float z)const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

XMFLOAT3 GeometryGenerator::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}
void GeometryGenerator::CreateLand(float width, float depth, unsigned int m, unsigned int n, MeshData& meshData)
{
	unsigned int vertexCount = m*n;
	unsigned int faceCount = (m - 1)*(n - 1) * 2;

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*depth;


	float dx = width / (n - 1);
	float dz = depth / (m - 1);
	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);
	meshData.Vertices.resize(vertexCount);
	for (unsigned int i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dz;
		for (unsigned int j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;
			float y = GetHillHeight(x, z);
			meshData.Vertices[i*n + j].Position = XMFLOAT3(x, y, z);
			meshData.Vertices[i*n + j].Normal = GetHillNormal(x,z);
			meshData.Vertices[i*n + j].TexC.x = j*du;
			meshData.Vertices[i*n + j].TexC.y = i*dv;
		}
	}

	meshData.Indices.resize(faceCount * 3);
	unsigned int k = 0;
	for (unsigned i = 0; i < m - 1; ++i)
	{
		for (unsigned int j = 0; j < n - 1; ++j)
		{
			meshData.Indices[k] = i * n + j;
			meshData.Indices[k + 1] = i * n + j + 1;
			meshData.Indices[k + 2] = (i + 1) * n + j;
			meshData.Indices[k + 3] = (i + 1) * n + j;
			meshData.Indices[k + 4] = i * n + j + 1;
			meshData.Indices[k + 5] = (i + 1) * n + j + 1;
			k += 6;
		}
	}

}


void GeometryGenerator::CreateGrid(float width, float depth, unsigned int m, unsigned int n, MeshData& meshData)
{
	unsigned int vertexCount = m*n;
	unsigned int faceCount = (m - 1)*(n - 1) * 2;

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*depth;


	float dx = width / (n - 1);
	float dz = depth / (m - 1);
	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);
	meshData.Vertices.resize(vertexCount);
	for (unsigned int i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dz;
		for (unsigned int j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;
			meshData.Vertices[i*n + j].Position = XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[i*n + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i*n + j].TexC.x = j*du;
			meshData.Vertices[i*n + j].TexC.y = i*dv;
		}
	}

	meshData.Indices.resize(faceCount * 3);
	unsigned int k = 0;
	for (unsigned i = 0; i < m - 1; ++i)
	{
		for (unsigned int j = 0; j < n - 1; ++j)
		{
			meshData.Indices[k] = i * n + j;
			meshData.Indices[k + 1] = i * n + j + 1;
			meshData.Indices[k + 2] = (i + 1) * n + j;
			meshData.Indices[k + 3] = (i + 1) * n + j;
			meshData.Indices[k + 4] = i * n + j + 1;
			meshData.Indices[k + 5] = (i + 1) * n + j + 1;
			k += 6;
		}
	}
}

void GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int stackCount, MeshData & meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	float stackHeight = height / stackCount;
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	unsigned int ringCount = stackCount + 1;

	for (unsigned int i = 0; i < ringCount; ++i)
	{
		float y = -0.5f*height + i*stackHeight;
		float r = bottomRadius + i*radiusStep;
		float dTheta = 2.0f*XM_PI / sliceCount;
		for (unsigned int j = 0; j <= sliceCount; ++j)
		{
			Vertex vertex;
			float c = cosf(j*dTheta);
			float s = sinf(j*dTheta);
			vertex.Position = XMFLOAT3(r*c, y, r*s);
			vertex.TexC.x = (float)j / sliceCount;
			vertex.TexC.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr*c, -height, dr*s);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.Normal, N);

			meshData.Vertices.push_back(vertex);
		}
	}

	unsigned int ringVertexCount = sliceCount + 1;
	for (unsigned int i = 0; i < stackCount; ++i)
	{
		for (unsigned int j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(i*ringVertexCount + j);
			meshData.Indices.push_back((i + 1)*ringVertexCount + j);
			meshData.Indices.push_back((i + 1)*ringVertexCount + j + 1);
			meshData.Indices.push_back(i*ringVertexCount + j);
			meshData.Indices.push_back((i + 1)*ringVertexCount + j + 1);
			meshData.Indices.push_back(i*ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(topRadius, height, sliceCount, meshData);
	BuildCylinderBottomCap(bottomRadius, height, sliceCount, meshData);
}

void GeometryGenerator::BuildCylinderTopCap(float topRadius, float height, unsigned int sliceCount,MeshData& meshData)
{
	unsigned int baseIndex = static_cast<unsigned int>(meshData.Vertices.size());
	float y = 0.5f*height;
	float dTheta = 2.0f*XM_PI / sliceCount;
	for (unsigned int i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius*cosf(i*dTheta);
		float z = topRadius*sinf(i*dTheta);
		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));


	//Index of center vertex
	unsigned int centerIndex = static_cast<unsigned int>(meshData.Vertices.size() - 1);
	for (unsigned int i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float height, unsigned int sliceCount, MeshData& meshData)
{
	unsigned int baseIndex = static_cast<unsigned int>(meshData.Vertices.size());
	float y = -0.5f*height;
	float dTheta = 2.0f*XM_PI / sliceCount;
	for (unsigned int i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius*cosf(i*dTheta);
		float z = bottomRadius*sinf(i*dTheta);
		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));


	//Index of center vertex
	unsigned int centerIndex = static_cast<unsigned int>(meshData.Vertices.size() - 1);
	for (unsigned int i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

void GeometryGenerator::CreateGeosphere(float radius, unsigned int numSubdivisions, MeshData& meshData)
{
	numSubdivisions = min(numSubdivisions, 5u);
	const float x = 0.525731f;
	const float z = 0.850651f;
	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-x, 0, z), XMFLOAT3(x, 0, z),
		XMFLOAT3(-x, 0, -z), XMFLOAT3(x, 0, -z),
		XMFLOAT3(0, z, x), XMFLOAT3(0, z, -x),
		XMFLOAT3(0, -z, x), XMFLOAT3(0, -z, -x),
		XMFLOAT3(z, x, 0), XMFLOAT3(-z, x, 0),
		XMFLOAT3(z, -x, 0), XMFLOAT3(-z, -x, 0),
	};

	unsigned long k[60] = 
	{
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4, 
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2, 
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
		10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7 
	};

	meshData.Vertices.resize(12);
	meshData.Indices.resize(60);
	for (std::vector<Vertex>::size_type i = 0; i < 12; ++i)
		meshData.Vertices[i].Position = pos[i];

	for (std::vector<Vertex>::size_type i = 0; i < 60; ++i)
		meshData.Indices[i] = k[i];
	for (unsigned int i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);
	for (std::vector<Vertex>::size_type i = 0; i < meshData.Vertices.size(); ++i)
	{
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.Vertices[i].Position));
		XMVECTOR p = radius*n;
		XMStoreFloat3(&meshData.Vertices[i].Position, p);
		XMStoreFloat3(&meshData.Vertices[i].Normal, n);

		// Derive texture coordinates from spherical coordinates.
		float theta = AngleFromXY(
			meshData.Vertices[i].Position.x,
			meshData.Vertices[i].Position.z);

		float phi = acosf(meshData.Vertices[i].Position.y / radius);

		meshData.Vertices[i].TexC.x = theta / XM_2PI;
		meshData.Vertices[i].TexC.y = phi / XM_PI;

		// Partial derivative of P with respect to theta
		meshData.Vertices[i].TangentU.x = -radius*sinf(phi)*sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +radius*sinf(phi)*cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
		XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));
	}


}

/*
void GeometryGenerator::Subdivide(MeshData& meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;


	meshData.Vertices.resize(0);
	meshData.Indices.resize(0);


	size_t numTris = inputCopy.Indices.size() / 3;
	for (unsigned int i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 0]];
		Vertex v1 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 1]];
		Vertex v2 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		Vertex m0, m1, m2;

		// For subdivision, we just care about the position component.  We derive the other
		// vertex components in CreateGeosphere.

		m0.Position = XMFLOAT3(
			0.5f*(v0.Position.x + v1.Position.x),
			0.5f*(v0.Position.y + v1.Position.y),
			0.5f*(v0.Position.z + v1.Position.z));

		m1.Position = XMFLOAT3(
			0.5f*(v1.Position.x + v2.Position.x),
			0.5f*(v1.Position.y + v2.Position.y),
			0.5f*(v1.Position.z + v2.Position.z));

		m2.Position = XMFLOAT3(
			0.5f*(v0.Position.x + v2.Position.x),
			0.5f*(v0.Position.y + v2.Position.y),
			0.5f*(v0.Position.z + v2.Position.z));

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5

		meshData.Indices.push_back(i * 6 + 0);
		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 5);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 2);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 1);
		meshData.Indices.push_back(i * 6 + 4);
	}
}
*/

void GeometryGenerator::Subdivide(MeshData& meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;


	meshData.Vertices.resize(0);
	meshData.Indices.resize(0);

	//       v1
	//       *
	//      / \
		//     /   \
	//  m0*-----*m1
//   / \   / \
	//  /   \ /   \
	// *-----*-----*
// v0    m2     v2

	size_t numTris = inputCopy.Indices.size()/3;
	for(unsigned int i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.Vertices[ inputCopy.Indices[i*3+0] ];
		Vertex v1 = inputCopy.Vertices[ inputCopy.Indices[i*3+1] ];
		Vertex v2 = inputCopy.Vertices[ inputCopy.Indices[i*3+2] ];

		//
		// Generate the midpoints.
		//

		Vertex m0, m1, m2;

		m0.Position = Lerp(v0.Position, v1.Position, 0.5f);
		m1.Position = Lerp(v1.Position, v2.Position, 0.5f);
		m2.Position = Lerp(v0.Position, v2.Position, 0.5f);

		m0.TexC = Lerp(v0.TexC, v1.TexC, 0.5f);
		m1.TexC = Lerp(v1.TexC, v2.TexC, 0.5f);
		m2.TexC = Lerp(v0.TexC, v2.TexC, 0.5f);

		XMStoreFloat3(&m0.Normal, XMVector3Normalize(XMLoadFloat3(&Lerp(v0.Normal, v1.Normal, 0.5f))));
		XMStoreFloat3(&m1.Normal, XMVector3Normalize(XMLoadFloat3(&Lerp(v1.Normal, v2.Normal, 0.5f))));
		XMStoreFloat3(&m2.Normal, XMVector3Normalize(XMLoadFloat3(&Lerp(v0.Normal, v2.Normal, 0.5f))));

		XMStoreFloat3(&m0.TangentU, XMVector3Normalize(XMLoadFloat3(&Lerp(v0.TangentU, v1.TangentU, 0.5f))));
		XMStoreFloat3(&m1.TangentU, XMVector3Normalize(XMLoadFloat3(&Lerp(v1.TangentU, v2.TangentU, 0.5f))));
		XMStoreFloat3(&m2.TangentU, XMVector3Normalize(XMLoadFloat3(&Lerp(v0.TangentU, v2.TangentU, 0.5f))));

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5

		meshData.Indices.push_back(i*6+0);
		meshData.Indices.push_back(i*6+3);
		meshData.Indices.push_back(i*6+5);

		meshData.Indices.push_back(i*6+3);
		meshData.Indices.push_back(i*6+4);
		meshData.Indices.push_back(i*6+5);

		meshData.Indices.push_back(i*6+5);
		meshData.Indices.push_back(i*6+4);
		meshData.Indices.push_back(i*6+2);

		meshData.Indices.push_back(i*6+3);
		meshData.Indices.push_back(i*6+1);
		meshData.Indices.push_back(i*6+4);
	}
}

void GeometryGenerator::CreateSphere(float radius, unsigned int sliceCount, unsigned int stackCount, MeshData& meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f*XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (unsigned int i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i*phiStep;

		// Vertices of ring.
		for (unsigned int j = 0; j <= sliceCount; ++j)
		{
			float theta = j*thetaStep;

			Vertex v;

			// spherical to cartesian
			v.Position.x = radius*sinf(phi)*cosf(theta);
			v.Position.y = radius*cosf(phi);
			v.Position.z = radius*sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius*sinf(phi)*sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius*sinf(phi)*cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

			v.TexC.x = theta / XM_2PI;
			v.TexC.y = phi / XM_PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (unsigned int i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices.push_back(0);
		meshData.Indices.push_back(i + 1);
		meshData.Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	unsigned int baseIndex = 1;
	unsigned int ringVertexCount = sliceCount + 1;
	for (unsigned int i = 0; i < stackCount - 2; ++i)
	{
		for (unsigned int j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(baseIndex + i*ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i*ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			meshData.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i*ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	unsigned int southPoleIndex = (unsigned int)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (unsigned int i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(southPoleIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}
void GeometryGenerator::CreateBox(float width, float height, float depth, UINT numSubdivisions, MeshData& meshData)
{
	//
	// Create the vertices.
	//

	Vertex v[24];

	float w2 = 0.5f*width;
	float h2 = 0.5f*height;
	float d2 = 0.5f*depth;

	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7]  = 5; i[8]  = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] =  9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices.assign(&i[0], &i[36]);

	// Put a cap on the number of subdivisions.
	numSubdivisions = __min(numSubdivisions, 5u);

	for(UINT i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);
}
