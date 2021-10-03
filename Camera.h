#ifndef CAMERA_H
#define CAMERA_H
using namespace DirectX;

class Camera {
public:
	Camera();
	~Camera() {}
	XMVECTOR GetPositionXM() const { return XMLoadFloat3(&mPosition); }
	XMFLOAT3 GetPosition() const { return mPosition; }
	void SetPosition(float x, float y, float z) { mPosition = XMFLOAT3(x, y, z); }
	void SetPosition(const XMFLOAT3& v) { mPosition = v; }

	XMVECTOR GetRightXM() const { return XMLoadFloat3(&mRight); }
	XMFLOAT3 GetRight() const { return mRight; }
	XMVECTOR GetUpXM() const { return XMLoadFloat3(&mUp); }
	XMFLOAT3 GetUp() const { return mUp; }
	XMVECTOR GetLookXM() const { return XMLoadFloat3(&mLook); }
	XMFLOAT3 GetLook() const { return mLook; }

	float GetNearZ() const { return mNearZ; }
	float GetFarZ() const { return mFarZ; }
	float GetAspect() const { return mAspect; }
	float GetFovY() const { return mFovY; }
	float GetFovX() const { return static_cast<float>(2.0f * atan(0.5f * GetNearWindowWidth() / mNearZ)); }

	float GetNearWindowWidth() const { return mAspect * mNearWindowHeight; }
	float GetNearWindowHeight() const { return mNearWindowHeight; }
	float GetFarWindowWidth() const { return mAspect * mFarWindowHeight; }
	float GetFarWindowHeight() const { return mFarWindowHeight; }

	void SetLens(float fovY, float aspect, float zn, float zf);

	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	XMMATRIX View() const { return XMLoadFloat4x4(&mView); }
	XMMATRIX Proj() const { return XMLoadFloat4x4(&mProj); }
	XMMATRIX ViewProj() const { return View() * Proj(); }

	void Strafe(float d);//move left/right
	void Walk(float d);//move forward/backward

	void Pitch(float angle);//head/eyes/camera look up/down rotate along x-axis
	void RotateY(float angle);// look left/right rotate along y-axis

	void UpdateViewMatrix();

private:
	XMFLOAT3 mPosition;//camera position relative to world coordinates
	XMFLOAT3 mRight;//x-axis
	XMFLOAT3 mUp;//y-axis
	XMFLOAT3 mLook;//z-axis

	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

};
#endif
