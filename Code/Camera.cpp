#include "Camera.h"
#include "GLWindow.h"
#include "Compare.h"

#include <iostream>

Camera::Camera() {
    fieldOfView = 60.0f;
    aspectRatio = 1.3f;
    nearPlane = 0.01f;
    farPlane = 1000.0f;
    width = 1.0f;
    height = 1.0f;

    worldMatrix = mat4();
    projectionMatrix = CalculateProjectionMatrix(fieldOfView, aspectRatio, nearPlane, farPlane);
    projectionMode = 0;
}

mat4 Camera::GetWorldMatrix() {
    return worldMatrix;
}

bool Camera::IsOrthoNormal() {
    vec3 right = vec3(worldMatrix._11, worldMatrix._12, worldMatrix._13);
    vec3 up = vec3(worldMatrix._21, worldMatrix._22, worldMatrix._23);
    vec3 forward = vec3(worldMatrix._31, worldMatrix._32, worldMatrix._33);

    if (!IsCloseTo1(DotProduct(right, right))) {
        return false; // X axis is not normal
    }

    if (!IsCloseTo1(DotProduct(up, up))) {
        return false; // Y axis is not normal
    }

    if (!IsCloseTo1(DotProduct(forward, forward))) {
        return false; // Z axis is not normal
    }

    if (!IsCloseTo0(DotProduct(forward, up))) {
        return false; // Not perpendicular
    }

    if (!IsCloseTo0(DotProduct(forward, right))) {
        return false; // Not perpendicular
    }

    if (!IsCloseTo0(DotProduct(right, up))) {
        return false; // Not perpendicular
    }

    return true;
}

void Camera::OrthoNormalize() {
    vec3 right = vec3(worldMatrix._11, worldMatrix._12, worldMatrix._13);
    vec3 up = vec3(worldMatrix._21, worldMatrix._22, worldMatrix._23);
    vec3 forward = vec3(worldMatrix._31, worldMatrix._32, worldMatrix._33);

    vec3 f = Normalize(forward);
    vec3 r = Normalize(CrossProduct(up, f));
    vec3 u = CrossProduct(f, r);

    worldMatrix = mat4(
        r.x, r.y, r.z, 0.0f,
        u.x, u.y, u.z, 0.0f,
        f.x, f.y, f.z, 0.0f,
        worldMatrix._41, worldMatrix._42, worldMatrix._43, 1.0f
    );
}

mat4 Camera::GetViewMatrix() {
    if (!IsOrthoNormal()) {
        OrthoNormalize();
    }

    mat4 inverse = Transpose(worldMatrix);
    inverse._41 = inverse._14 = 0.0f;
    inverse._42 = inverse._24 = 0.0f;
    inverse._43 = inverse._34 = 0.0f;

    vec3 right = vec3(worldMatrix._11, worldMatrix._12, worldMatrix._13);
    vec3 up = vec3(worldMatrix._21, worldMatrix._22, worldMatrix._23);
    vec3 forward = vec3(worldMatrix._31, worldMatrix._32, worldMatrix._33);
    vec3 position = vec3(worldMatrix._41, worldMatrix._42, worldMatrix._43);

    inverse._41 = -DotProduct(right, position);
    inverse._42 = -DotProduct(up, position);
    inverse._43 = -DotProduct(forward, position);

    return inverse;
}

float Camera::GetAspect() {
    return aspectRatio;
}

mat4 Camera::GetProjectionMatrix() {
    return projectionMatrix;
}

void Camera::Resize(int width, int height) {
    aspectRatio = (float)width / (float)height;

    if (projectionMode == 0) { // Perspective
        projectionMatrix = CalculateProjectionMatrix(fieldOfView, aspectRatio, nearPlane, farPlane);
    }
    else if (projectionMode == 1) { // Ortho
        width = (float)width;
        height = (float)height;

        float halfW = width * 0.5f;
        float halfH = height * 0.5f;

        projectionMatrix = CalculateOrthoMatrix(-halfW, halfW, halfH, -halfH, nearPlane, farPlane);
    }
}

bool Camera::IsOrthographic() {
    return projectionMode == 1;
}

bool Camera::IsPerspective() {
    return projectionMode == 0;
}

void Camera::SetPerspective(float fov, float aspect, float near, float far) {
    fieldOfView = fov;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;

    projectionMatrix = CalculateProjectionMatrix(fov, aspect, near, far);
    projectionMode = 0;
}

void Camera::SetOrthographic(float width, float height, float near, float far) {
    this->width = width;
    this->height = height;
    nearPlane = near;
    farPlane = far;

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    projectionMatrix = CalculateOrthoMatrix(-halfW, halfW, halfH, -halfH, near, far);
    projectionMode = 1;
}

void Camera::SetProjectionMatrix(const mat4& projection) {
    projectionMatrix = projection;
    projectionMode = 2;
}

void Camera::SetWorldMatrix(const mat4& view) {
    worldMatrix = view;
}

Camera CreatePerspectiveCamera(float fov, float aspect, float near, float far) {
    Camera result;
    result.SetPerspective(fov, aspect, near, far);
    return result;
}

Camera CreateOrthographicCamera(float width, float height, float near, float far) {
    Camera result;
    result.SetOrthographic(width, height, near, far);
    return result;
}

OrbitCamera::OrbitCamera() {
    targetPosition = vec3(0, 0, 0);
    zoomDistance = 10.0f;
    zoomSpeed = 200.0f;
    rotationSpeed = vec2(250.0f, 120.0f);
    yRotationLimit = vec2(-20.0f, 80.0f);
    zoomDistanceLimit = vec2(3.0f, 15.0f);
    currentRotation = vec2(0, 0);
    panSpeed = vec2(180.0f, 180.0f);
}

void OrbitCamera::Rotate(const vec2& deltaRotation, float deltaTime) {
    currentRotation.x += deltaRotation.x * rotationSpeed.x * zoomDistance * deltaTime;
    currentRotation.y += deltaRotation.y * rotationSpeed.y * zoomDistance * deltaTime;

    currentRotation.x = ClampAngle(currentRotation.x, -360, 360);
    currentRotation.y = ClampAngle(currentRotation.y, yRotationLimit.x, yRotationLimit.y);
}

void OrbitCamera::Zoom(float deltaZoom, float deltaTime) {
    zoomDistance = zoomDistance + deltaZoom * zoomSpeed * deltaTime;
    if (zoomDistance < zoomDistanceLimit.x) {
        zoomDistance = zoomDistanceLimit.x;
    }
    if (zoomDistance > zoomDistanceLimit.y) {
        zoomDistance = zoomDistanceLimit.y;
    }
}

void OrbitCamera::Pan(const vec2& deltaPan, float deltaTime) {
    vec3 right(worldMatrix._11, worldMatrix._12, worldMatrix._13);

    // Pan along the X-axis in local space
    targetPosition = targetPosition - (right * (deltaPan.x * panSpeed.x * deltaTime));
    // Pan along the Y-axis in global space
    targetPosition = targetPosition + (vec3(0, 1, 0) * (deltaPan.y * panSpeed.y * deltaTime));

    // Reset zoom to allow infinite zooming after a motion
    // This part of the code is not in the book!
    float midZoom = zoomDistanceLimit.x + (zoomDistanceLimit.y - zoomDistanceLimit.x) * 0.5f;
    zoomDistance = midZoom - zoomDistance;
    vec3 rotation = vec3(currentRotation.y, currentRotation.x, 0);
    mat3 orientation = CalculateRotationMatrix(rotation.x, rotation.y, rotation.z);
    vec3 direction = MultiplyVector(vec3(0.0f, 0.0f, -zoomDistance), orientation);
    targetPosition = targetPosition - direction;
    zoomDistance = midZoom;
}

void OrbitCamera::UpdateCamera(float deltaTime) {
    vec3 rotation = vec3(currentRotation.y, currentRotation.x, 0);
    mat3 orientation = CalculateRotationMatrix(rotation.x, rotation.y, rotation.z);
    vec3 direction = MultiplyVector(vec3(0.0f, 0.0f, -zoomDistance), orientation);
    vec3 position = direction + targetPosition;
    worldMatrix = FastInverse(LookAt(position, targetPosition, vec3(0, 1, 0)));
}

float OrbitCamera::ClampAngle(float angle, float minAngle, float maxAngle) {
    while (angle < -360) {
        angle += 360;
    }
    while (angle > 360) {
        angle -= 360;
    }
    if (angle < minAngle) {
        angle = minAngle;
    }
    if (angle > maxAngle) {
        angle = maxAngle;
    }
    return angle;
}

Frustum Camera::GetFrustum() {
    Frustum result;

    mat4 viewProjection = GetViewMatrix() * GetProjectionMatrix();

    vec3 col1(viewProjection._11, viewProjection._21, viewProjection._31);
    vec3 col2(viewProjection._12, viewProjection._22, viewProjection._32);
    vec3 col3(viewProjection._13, viewProjection._23, viewProjection._33);
    vec3 col4(viewProjection._14, viewProjection._24, viewProjection._34);

    // Find plane magnitudes
    result.left.normal = col4 + col1;
    result.right.normal = col4 - col1;
    result.bottom.normal = col4 + col2;
    result.top.normal = col4 - col2;
    result.nearPlane.normal = col3;
    result.farPlane.normal = col4 - col3;

    // Find plane distances
    result.left.distance = viewProjection._44 + viewProjection._41;
    result.right.distance = viewProjection._44 - viewProjection._41;
    result.bottom.distance = viewProjection._44 + viewProjection._42;
    result.top.distance = viewProjection._44 - viewProjection._42;
    result.nearPlane.distance = viewProjection._43;
    result.farPlane.distance = viewProjection._44 - viewProjection._43;

    // Normalize all 6 planes
    for (int i = 0; i < 6; ++i) {
        float magnitude = 1.0f / Magnitude(result.planes[i].normal);
        Normalize(result.planes[i].normal);
        result.planes[i].distance *= magnitude;
    }

    return result;
}

void OrbitCamera::PrintDebugInfo() {
    std::cout << "Target Position: (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")\n";
    std::cout << "Zoom Distance: " << zoomDistance << "\n";
    std::cout << "Rotation: (" << currentRotation.x << ", " << currentRotation.y << ")\n";
}

void OrbitCamera::SetTargetPosition(const vec3& newTarget) {
    targetPosition = newTarget;
}

void OrbitCamera::SetZoomDistance(float zoom) {
    zoomDistance = zoom;
}

void OrbitCamera::SetRotation(const vec2& rotation) {
    currentRotation = rotation;
}