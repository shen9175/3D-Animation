#ifndef D3DANIMATION_H
#define D3DANIMATION_H

class D3DAnimation : public D3DBase11 {
	friend Object3DList;
public:
	D3DAnimation(HINSTANCE hInstance, bool enableMSAA, UINT8 MSAAcount, int width, int height, D3D_DRIVER_TYPE type);
	~D3DAnimation();

	bool Init();
	void OnResize();
	void Update(float dt);
	void Draw();
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyDown(WPARAM wParam);
private:
	Camera Cam;
	DirectionalLight mDirLights[3];
	PointLight mPointLights[10];
	SpotLight mSpotLights[2];
	bool dirLightsw[3];
	bool pointLightsw[10];
	bool SpotLightsw[2];
	bool fogenable;
	bool renderWire;
	Object3DList* list;
	POINT mLastMousePos;
};

#endif