//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#include "PCH.h"

#include "Camera.h"

#include "Utility.h"

namespace SampleFramework11
{

//=========================================================================================
// Camera
//=========================================================================================
Camera::Camera(float nearClip, float farClip) : nearZ(nearClip),
                                                farZ(farClip)
{
    _ASSERT(nearZ > 0 && nearZ < farZ);
    _ASSERT(farZ > 0 && farZ > nearZ);

    world = XMMatrixIdentity();
    view = XMMatrixIdentity();
    position = XMLoadFloat3(&XMFLOAT3(0, 0, 0));
    orientation = XMQuaternionIdentity();
}

Camera::~Camera()
{

}

void Camera::WorldMatrixChanged()
{
    XMVECTOR det;
    view = XMMatrixInverse(&det, world);
    viewProjection = XMMatrixMultiply(view, projection);
}

XMVECTOR Camera::Forward() const
{
    return ForwardVec(world);
}

XMVECTOR Camera::Back() const
{
    return BackVec(world);
}

XMVECTOR Camera::Up() const
{
    return UpVec(world);
}

XMVECTOR Camera::Down() const
{
    return DownVec(world);
}

XMVECTOR Camera::Right() const
{
    return RightVec(world);
}

XMVECTOR Camera::Left() const
{
    return LeftVec(world);
}

void Camera::SetLookAt(const XMFLOAT3 &eye, const XMFLOAT3 &lookAt, const XMFLOAT3 &up)
{
    XMVECTOR det;
    view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt), XMLoadFloat3(&up));
    world = XMMatrixInverse(&det, view);
    position = XMLoadFloat3(&eye);
    orientation = XMQuaternionRotationMatrix(world);
}

void Camera::SetWorldMatrix(const XMMATRIX& newWorld)
{
    world = newWorld;
    position = TranslationVec(world);
    orientation = XMQuaternionRotationMatrix(world);

    WorldMatrixChanged();
}

void Camera::SetPosition(const XMVECTOR& newPosition)
{
    position = newPosition;
    SetTranslationVec(world, newPosition);

    WorldMatrixChanged();
}

void Camera::SetOrientation(const XMVECTOR& newOrientation)
{
    world = XMMatrixRotationQuaternion(newOrientation);
    orientation = newOrientation;
    SetTranslationVec(world, position);

    WorldMatrixChanged();
}

void Camera::SetNearClip(float newNearClip)
{
    nearZ = newNearClip;
    CreateProjection();
}

void Camera::SetFarClip(float newFarClip)
{
    farZ = newFarClip;
    CreateProjection();
}

//=========================================================================================
// OrthographicCamera
//=========================================================================================

OrthographicCamera::OrthographicCamera(float minX, float minY, float maxX,
                                       float maxY, float nearClip, float farClip) : Camera(nearClip, farClip),
                                                                                    xMin(minX),
                                                                                    yMin(minY),
                                                                                    xMax(maxX),
                                                                                    yMax(maxY)

{
    _ASSERT(xMax > xMin && yMax > yMin);

    CreateProjection();
}

OrthographicCamera::~OrthographicCamera()
{

}

void OrthographicCamera::CreateProjection()
{
    projection = XMMatrixOrthographicOffCenterLH(xMin, xMax, yMin, yMax, nearZ, farZ);
    viewProjection = XMMatrixMultiply(view, projection);
}

void OrthographicCamera::SetMinX(float minX)
{
    xMin = minX;
    CreateProjection();
}

void OrthographicCamera::SetMinY(float minY)
{
    yMin = minY;
    CreateProjection();
}

void OrthographicCamera::SetMaxX(float maxX)
{
    xMax = maxX;
    CreateProjection();
}

void OrthographicCamera::SetMaxY(float maxY)
{
    yMax = maxY;
    CreateProjection();
}

//=========================================================================================
// PerspectiveCamera
//=========================================================================================

PerspectiveCamera::PerspectiveCamera(float aspectRatio, float fieldOfView,
                                     float nearClip, float farClip) :   Camera(nearClip, farClip),
                                                                        aspect(aspectRatio),
                                                                        fov(fieldOfView)
{
    _ASSERT(aspectRatio > 0);
    _ASSERT(fieldOfView > 0 && fieldOfView <= 3.14159f);
    CreateProjection();
}

PerspectiveCamera::~PerspectiveCamera()
{

}

void PerspectiveCamera::SetAspectRatio(float aspectRatio)
{
    aspect = aspectRatio;
    CreateProjection();
}

void PerspectiveCamera::SetFieldOfView(float fieldOfView)
{
    fov = fieldOfView;
    CreateProjection();
}

void PerspectiveCamera::CreateProjection()
{
    projection = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
    viewProjection = XMMatrixMultiply(view, projection);
}

//=========================================================================================
// FirstPersonCamera
//=========================================================================================

FirstPersonCamera::FirstPersonCamera(float aspectRatio, float fieldOfView,
                                     float nearClip, float farClip) : PerspectiveCamera(aspectRatio, fieldOfView,
                                                                                                nearClip, farClip),
                                                                                                xRot(0),
                                                                                                yRot(0)
{

}

FirstPersonCamera::~FirstPersonCamera()
{

}

void FirstPersonCamera::SetXRotation(float xRotation)
{
    xRot = Clamp(xRotation, -PiOver2, PiOver2);
    SetOrientation(XMQuaternionRotationRollPitchYaw(xRot, yRot, 0));
}

void FirstPersonCamera::SetYRotation(float yRotation)
{
    yRot = XMScalarModAngle(yRotation);
    SetOrientation(XMQuaternionRotationRollPitchYaw(xRot, yRot, 0));
}

}