//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#pragma once

#include "PCH.h"

namespace SampleFramework11
{

// Abstract base class for camera types
class Camera
{

protected:

    XMMATRIX view;
    XMMATRIX projection;
    XMMATRIX viewProjection;

    XMMATRIX world;
    XMVECTOR position;
    XMVECTOR orientation;

    float nearZ;
    float farZ;

    virtual void CreateProjection() = 0;
    void WorldMatrixChanged();

public:

    Camera(float nearZ, float farZ);
    ~Camera();

    const XMMATRIX& ViewMatrix() const { return view; };
    const XMMATRIX& ProjectionMatrix() const { return projection; };
    const XMMATRIX& ViewProjectionMatrix() const { return viewProjection; };
    const XMMATRIX& WorldMatrix() const { return world; };
    const XMVECTOR& Position() const { return position; };
    const XMVECTOR& Orientation() const { return orientation; };
    const float& NearClip() const { return nearZ; };
    const float& FarClip() const { return farZ; };

    XMVECTOR Forward() const;
    XMVECTOR Back() const;
    XMVECTOR Up() const;
    XMVECTOR Down() const;
    XMVECTOR Right() const;
    XMVECTOR Left() const;

    void SetLookAt(const XMFLOAT3& eye, const XMFLOAT3& lookAt, const XMFLOAT3& up);
    void SetWorldMatrix(const XMMATRIX& newWorld);
    void SetPosition(const XMVECTOR& newPosition);
    void SetOrientation(const XMVECTOR& newOrientation);
    void SetNearClip(float newNearClip);
    void SetFarClip(float newFarClip);
};

// Camera with an orthographic projection
class OrthographicCamera : public Camera
{

protected:

    float xMin;
    float xMax;
    float yMin;
    float yMax;

    virtual void CreateProjection();

public:

    OrthographicCamera(float minX, float minY, float maxX, float maxY, float nearClip, float farClip);
    ~OrthographicCamera();

    float MinX() const { return xMin; };
    float MinY() const { return yMin; };
    float MaxX() const { return xMax; };
    float MaxY() const { return yMax; };

    void SetMinX(float minX);
    void SetMinY(float minY);
    void SetMaxX(float maxX);
    void SetMaxY(float maxY);
};

// Camera with a perspective projection
class PerspectiveCamera : public Camera
{

protected:

    float aspect;
    float fov;

    virtual void CreateProjection();

public:

    PerspectiveCamera(float aspectRatio, float fieldOfView, float nearClip, float farClip);
    ~PerspectiveCamera();

    float AspectRatio() const { return aspect; };
    float FieldOfView() const { return fov; };

    void SetAspectRatio(float aspectRatio);
    void SetFieldOfView(float fieldOfView);
};

// Perspective camera that rotates about Z and Y axes
class FirstPersonCamera : public PerspectiveCamera
{

protected:

    float xRot;
    float yRot;

public:

    FirstPersonCamera(float aspectRatio, float fieldOfView, float nearClip, float farClip);
    ~FirstPersonCamera();

    float XRotation() const { return xRot; };
    float YRotation() const { return yRot; };

    void SetXRotation(float xRotation);
    void SetYRotation(float yRotation);
};

}