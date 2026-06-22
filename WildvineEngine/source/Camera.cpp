#include "EngineUtilities\Utilities\Camera.h"

Camera::Camera() {
	XMStoreFloat4x4(&m_view, XMMatrixIdentity());
	XMStoreFloat4x4(&m_proj, XMMatrixIdentity());
}

void
Camera::setLens(float fovYRadians,
	float aspectRatio,
	float nearPlane,
	float farPlane) {
	m_fovY = fovYRadians;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;

	XMMATRIX proj = XMMatrixPerspectiveFovLH(m_fovY, m_aspectRatio, m_nearPlane, m_farPlane);
	XMStoreFloat4x4(&m_proj, proj);
}

void
Camera::setPosition(float x, float y, float z) {
	m_position = EU::Vector3(x, y, z);
	m_viewDirty = true;
}

void
Camera::setPosition(const EU::Vector3& pos) {
	m_position = pos;
	m_viewDirty = true;
}

void
Camera::lookAt(const EU::Vector3& pos, const EU::Vector3& target, const EU::Vector3& up) {
	m_position = pos;
	XMVECTOR P = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);
	XMVECTOR T = XMVectorSet(target.x, target.y, target.z, 1.0f);
	XMVECTOR U = XMVectorSet(up.x, up.y, up.z, 0.0f);

	XMVECTOR F = XMVector3Normalize(XMVectorSubtract(T, P));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(U, F));
	XMVECTOR Up = XMVector3Cross(F, R);

	m_forward = FromXM(F);
	m_right = FromXM(R);
	m_up = FromXM(Up);
}

void
Camera::walk(float d) {
	XMVECTOR F = XMVectorSet(m_forward.x, m_forward.y, m_forward.z, 0.0f);
	XMVECTOR P = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);
	//P = XMVectorAdd(P, XMVectorScale(F, d));
	P += d * F;
	m_position = FromXM(P);
	m_viewDirty = true;
}

void
Camera::strafe(float d) {
	XMVECTOR R = XMVectorSet(m_right.x, m_right.y, m_right.z, 0.0f);
	XMVECTOR P = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);
	//P = XMVectorAdd(P, XMVectorScale(R, d));
	P += d * R;
	m_position = FromXM(P);
	m_viewDirty = true;
}

void
Camera::yaw(float radians) {
	// Rotación alrededor del eje Y global
	XMMATRIX rot = XMMatrixRotationY(radians);

	XMVECTOR R = XMVectorSet(m_right.x, m_right.y, m_right.z, 0.0f);
	XMVECTOR U = XMVectorSet(m_up.x, m_up.y, m_up.z, 0.0f);
	XMVECTOR F = XMVectorSet(m_forward.x, m_forward.y, m_forward.z, 0.0f);

	R = XMVector3TransformNormal(R, rot);
	U = XMVector3TransformNormal(U, rot);
	F = XMVector3TransformNormal(F, rot);

	m_right = FromXM(R);
	m_up = FromXM(U);
	m_forward = FromXM(F);

	m_viewDirty = true;
}

void
Camera::pitch(float radians) {
	// Rotación alrededor del eje Right local
	XMVECTOR R = XMVectorSet(m_right.x, m_right.y, m_right.z, 0.0f);
	XMMATRIX rot = XMMatrixRotationAxis(R, radians);

	XMVECTOR U = XMVectorSet(m_up.x, m_up.y, m_up.z, 0.0f);
	XMVECTOR F = XMVectorSet(m_forward.x, m_forward.y, m_forward.z, 0.0f);

	U = XMVector3TransformNormal(U, rot);
	F = XMVector3TransformNormal(F, rot);

	m_up = FromXM(U);
	m_forward = FromXM(F);

	m_viewDirty = true;
}

void
Camera::updateViewMatrix() {
	if (!m_viewDirty) return;
	XMVECTOR R = XMVectorSet(m_right.x, m_right.y, m_right.z, 0.0f);
	XMVECTOR U = XMVectorSet(m_up.x, m_up.y, m_up.z, 0.0f);
	XMVECTOR F = XMVectorSet(m_forward.x, m_forward.y, m_forward.z, 0.0f);
	XMVECTOR P = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);

	// Re-ortonormalizar (para evitar drift por floats)
	F = XMVector3Normalize(F);
	R = XMVector3Normalize(XMVector3Cross(U, F));
	U = XMVector3Cross(F, R);

	m_forward = FromXM(F);
	m_right = FromXM(R);
	m_up = FromXM(U);

	// View (LH) mirando en dirección forward | View = LookToLH(pos, pos + forward, up)
	XMMATRIX view = XMMatrixLookToLH(P, F, U);
	XMStoreFloat4x4(&m_view, view);
	m_viewDirty = false;
}