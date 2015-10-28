#include <d3d11_2.h>
#include <DirectXMath.h>
#include "Camera.h"

Camera::Camera() : mPosition(0.0f, 0.0f, 0.0f), mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mLook(0.0f, 0.0f, 1.0f) {
	SetLens(0.25f * XM_PI, 1.0f, 1.0f, 1000.0f);
}

void Camera::SetLens(float fovY, float aspect, float nz, float fz) {
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = nz;
	mFarZ = fz;

	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp) {
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
}
void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up) {
	LookAt(XMLoadFloat3(&pos), XMLoadFloat3(&target), XMLoadFloat3(&up));
}

void Camera::Strafe(float d) {
	XMVECTOR translation = XMVectorReplicate(d);
	XMVECTOR x_axis = XMLoadFloat3(&mRight);
	XMVECTOR pos = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(translation, x_axis, pos));
	//translation along x-axis, pos += d * x-axis;
}
void Camera::Walk(float d) {
	XMVECTOR translation = XMVectorReplicate(d);
	XMVECTOR z_axis = XMLoadFloat3(&mLook);
	XMVECTOR pos = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(translation, z_axis, pos));
	//translation along z-axis: pos += d * z-axis;
}

void Camera::Pitch(float angle) {
	//rotate up/down along camera x-axis, but when caculate rotation, it use world vector which is mRight vector
	//XMMatrixRotationX is rotate as world x ? check this
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

}
void Camera::RotateY(float angle) {
	XMMATRIX R = XMMatrixRotationY(angle);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
}

void Camera::UpdateViewMatrix() {
	XMVECTOR R = XMLoadFloat3(&mRight);
	XMVECTOR U = XMLoadFloat3(&mUp);
	XMVECTOR L = XMLoadFloat3(&mLook);
	XMVECTOR P = XMLoadFloat3(&mPosition);

	// re-orthogonal each other
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));
	R = XMVector3Cross(U, L);

	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	XMStoreFloat3(&mLook, L);

	mView(0, 0) = mRight.x;
	mView(1, 0) = mRight.y;
	mView(2, 0) = mRight.z;
	mView(3, 0) = x;

	mView(0, 1) = mUp.x;
	mView(1, 1) = mUp.y;
	mView(2, 1) = mUp.z;
	mView(3, 1) = y;

	mView(0, 2) = mLook.x;
	mView(1, 2) = mLook.y;
	mView(2, 2) = mLook.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;
}