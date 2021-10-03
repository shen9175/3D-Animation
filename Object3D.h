#ifndef OBJECT3D_H
#define OBJECT3D_H
using namespace DirectX;

struct PosNormalTex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};
struct PosNormalTexTan
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 TangentU;
};

struct PosNormalTexTanAnimation
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 TangentU;
	XMFLOAT3 Weights;
	BYTE BoneIndices[4];
};
struct Mirror {
	bool isMirror;
	XMFLOAT4 plane;
	Mirror() : isMirror(false), plane({ 0,0,0,0 }) {}
	Mirror(bool is, XMFLOAT4 p) : isMirror(is), plane(p) {}
};

struct ObjectInstance_Custom_DESC {
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	const std::vector<PosNormalTexTan> vertices;
	const std::vector<unsigned int> indices;
	Mirror mirror;
};
struct ObjectInstance_M3dModelList_DESC{
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	std::string modelName;
	std::string modelFilename;
	std::string texturePath;
	std::string VShaderName;
	std::string PShaderName;
	std::string LayoutName;
	std::string vertexTypeName;
	Mirror mirror;
};

struct ObjectInstance_LANDLIST_DESC {
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	LAND_DESC desc;
};


struct ObjectInstance_GRIDLIST_DESC {
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	GRID_DESC desc;
	Mirror mirror;
};

struct ObjectInstance_CYLINDERLIST_DESC {
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	CYLINDER_DESC desc;
};

struct ObjectInstance_GEOSPHERELIST_DESC {
	const std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	GEOSPHERE_DESC desc;
};

struct ObjectInstance_SPHERELIST_DESC {
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	SPHERE_DESC desc;
};

struct ObjectInstance_BOXLIST_DESC {
	std::string instanceName;
	XMFLOAT3 translation;
	XMFLOAT4X4 rotation;
	XMFLOAT3 scale;
	XMFLOAT3 texTransformation;
	const std::string modelname;
	Material material;
	const std::string texturePath;
	const std::string diffuseMap;
	const std::string normalMap;
	const std::string VShaderName;
	const std::string PShaderName;
	const std::string LayoutName;
	const std::string vertexTypeName;
	BOX_DESC desc;
};


struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (Direction, Spot)
	XMFLOAT3 Direction;
	float Spot;

	// Packed into 4D vector: (Att, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};


struct PerFrameCB {
	//XMFLOAT4X4 View;
	//XMFLOAT4X4 InvView;
	//XMFLOAT4X4 Proj;
	//XMFLOAT4X4 InvProj;
	XMFLOAT4X4 ViewProj;

	DirectionalLight DirLights[3];
	PointLight PointLights[10];
	SpotLight SpotLights[2];
	XMFLOAT4 ClipPlane;
	XMFLOAT3 EyePosW;
	float pad;

	XMFLOAT4 gFogColor;
	float gFogStart;
	float gFogRange;
};

struct PerObjectCB {
	XMFLOAT4X4 World;
	XMFLOAT4X4 WorldInvTranspose;
	XMFLOAT4X4 TexTransform;
	Material Mat;
};

struct ControlCB {
	unsigned int DirLightCount;
	unsigned int PointLightCount;
	unsigned int SpotLightCount;
	unsigned int UseTexture;
	unsigned int AlphaClip;
	unsigned int FogEnable;
	unsigned int ReflectionEnable;
	unsigned int ClipPlaneEnable;
};




class Object3D {
friend class Object3DList;
friend struct Object3DInstance;
public:
	Object3D(const std::string& modelFileName, const std::string& texturePath, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const Mirror& mirror);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const std::vector<PosNormalTexTan>& vertices, const std::vector<unsigned int>& indices, const Mirror& mirror);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const LAND_DESC& desc);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const GRID_DESC& desc, const Mirror& mirror);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const CYLINDER_DESC& desc);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const GEOSPHERE_DESC& desc);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const SPHERE_DESC& desc);
	Object3D(const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const BOX_DESC& desc);
	~Object3D() { delete[]Vertices; }
private:
	void ObjectInitialHelper(const size_t& VertexCount, const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr);
	std::vector<M3dMaterial>::size_type SubsetCount;
	std::vector<Material> Mat;
	void* Vertices;
	std::vector<char> vsarray;//hold vertex byte data
	std::vector<unsigned int> Indices;
	std::vector<Subset> Subsets;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;
	AnimationData Animations;
	Mirror mirror;
	size_t VertexCountBytes;
	size_t VertexOffsetBytes;
	size_t IndexOffset;
	size_t IndexCount;
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;
	ID3D11InputLayout* InputLayout;
	std::string vertextype;
	size_t typesize;
};
//there is only 1 model data in the memory, and could be multiply model instance, the different only the position, orientation etc
//to save some memory, we create a Object3DInstance class which has one model pointer with different position etc parameters
struct Object3DInstance {
	Object3DInstance()  { XMStoreFloat4x4(&mWorld, XMMatrixIdentity()); mTexTransform = {1.0f,1.0f,1.0f}; TimePos = 0.0f; ClipName = "Take1"; }
	Object3D* Model;
	float TimePos;
	std::string ClipName;
	XMFLOAT4X4 mWorld;//matrix stores in class/struct need use xmfloat4x4, xmmatrix need 16bytes alignment
	XMFLOAT3 mTexTransform;
	std::vector<XMFLOAT4X4> FinalTransforms;
	//void ObjectInstanceDraw(ID3D11Device*, ID3D11DeviceContext*, ID3D11Buffer* VB, ID3D11Buffer* IB, ID3D11Buffer*,ID3D11Buffer*,Camera& mCam, ConstantBuffer<PerObjectCB>& mPerObjectCB, std::unordered_map <unsigned short, ConstantBufferVector<XMFLOAT4X4>*> & mPerAnimationCB);
	void ObjectInstanceDraw(ID3D11Device*, ID3D11DeviceContext*, ID3D11Buffer* VB, ID3D11Buffer* IB, Camera& mCam, ConstantBuffer<PerObjectCB>& mPerObjectCB, std::unordered_map <unsigned short, ConstantBufferVector<XMFLOAT4X4>*> & mPerAnimationCB);
	void MoveTo(CXMMATRIX translation, CXMMATRIX scale, CXMMATRIX rotation);
	void SetTexTransform(XMFLOAT3 transTex) { mTexTransform = transTex; }
	void Update(float dt);
};

class D3DAnimation;
class Object3DList {
	friend class Object3DList;
public:
	Object3DList(D3DAnimation* d3dptr);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_M3dModelList_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_Custom_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_LANDLIST_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_GRIDLIST_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_CYLINDERLIST_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_GEOSPHERELIST_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_SPHERELIST_DESC> objectlist);
	Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_BOXLIST_DESC> objectlist);
	void AddM3dModelList(std::vector<ObjectInstance_M3dModelList_DESC>objectlist);
	void AddCustomList(std::vector<ObjectInstance_Custom_DESC> objectlist);
	void AddLandList(std::vector<ObjectInstance_LANDLIST_DESC> objectlist);
	void AddGridList(std::vector<ObjectInstance_GRIDLIST_DESC> objectlist);
	void AddCylinderList(std::vector<ObjectInstance_CYLINDERLIST_DESC> objectlist);
	void AddGeoSphereList(std::vector<ObjectInstance_GEOSPHERELIST_DESC> objectlist);
	void AddSphereList(std::vector<ObjectInstance_SPHERELIST_DESC> objectlist);
	void AddBoxist(std::vector<ObjectInstance_BOXLIST_DESC> objectlist);
	~Object3DList();
	Object3DInstance* getPlayer(std::string name) { if (instancelist.find(name) == instancelist.cend()) { return nullptr; } else { return instancelist.at(name); } }
	void Init();
	void FrameDraw();
	void Update(float dt);
private:
	void MirrorReflectionDraw(std::deque<Object3DInstance*>& mirrorlist, const Object3DInstance* currentMirror, const unsigned char recursionlevel);
	//void DrawReflectionsofCurrentMirror(const std::vector<Object3DInstance*>& mirrorlist, size_t currentIndex, const unsigned char);
	void CreateVideoRAMBuffer();
	void AddObjectList(Object3D* temp, const std::string& modelname);
	void AddInstanceList(const std::string& instanceName, const std::string& modelname, const XMFLOAT3& translation, const XMFLOAT4X4& rotation, const XMFLOAT3& scale, const XMFLOAT3& texTransform);
	D3DAnimation* app;
	Shaders* shaderMgr;
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	//ID3D11Buffer* VertexOutputDebugbuffer;
	//ID3D11Buffer* IndexOutputDebugbuffer;
	TextureMangement texMgr;
	ConstantBuffer<PerObjectCB> mPerObjectCB;
	ConstantBuffer<PerFrameCB> mPerFrameCB;
	ConstantBuffer<ControlCB> mControlCB;
	std::unordered_map<unsigned short, ConstantBufferVector<XMFLOAT4X4>*> PerAnimationCB;
	size_t mLastVertiesOffsetBytes;
	size_t mLastVertiesCountBytes;
	size_t mLastIndicesOffset;
	size_t mLastIndicesCount;
	size_t mTotalVertexCountBytes;
	size_t mTotalIndexCount;
	std::unordered_map<std::string, Object3D*> modellist;
	std::unordered_map<std::string, Object3DInstance*> instancelist;
	std::deque<Object3DInstance*> mirrorlist;
	std::queue<Object3D*> modelOrder;
};

#endif