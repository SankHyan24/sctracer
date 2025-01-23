#include <core/camera.hpp>
using namespace scTracer::Core;

Camera::Camera(glm::vec3 position, glm::vec3 lookAt, float fovDegree) {
    mPosition = position;
    mFront = glm::normalize(lookAt - position);
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    pitch = Utils::mathUtils::radians2degrees(asin(mFront.y));
    yaw = Utils::mathUtils::radians2degrees(atan2(mFront.z, mFront.x));
    radius = glm::length(lookAt - position);

    mFov = Utils::mathUtils::degrees2radians(fovDegree);
    mFocalDist = 0.01f;
    mAperture = 0.0f;

    updateCameraVectors();
}

Camera::Camera(glm::mat4 transform, float fovDegree) {
    mPosition = glm::vec3(transform[3]);
    mFront = glm::normalize(glm::vec3(transform[2]));
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    pitch = Utils::mathUtils::radians2degrees(asin(mFront.y));
    yaw = Utils::mathUtils::radians2degrees(atan2(mFront.z, mFront.x));
    radius = glm::length(mPosition);

    mFov = Utils::mathUtils::degrees2radians(fovDegree);
    mFocalDist = 0.01f;
    mAperture = 0.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(Utils::mathUtils::degrees2radians(pitch)) * cos(Utils::mathUtils::degrees2radians(yaw));
    front.y = sin(Utils::mathUtils::degrees2radians(pitch));
    front.z = cos(Utils::mathUtils::degrees2radians(pitch)) * sin(Utils::mathUtils::degrees2radians(yaw));
    mFront = glm::normalize(front);

    mRight = glm::normalize(glm::cross(mFront, worldUp));
    mUp = glm::normalize(glm::cross(mRight, mFront));

    mViewMat = glm::lookAt(mPosition, mPosition + mFront, mUp);
    mProjMat = glm::perspective(mFov, 1.0f, 0.1f, 100.0f);
    mVPMat = mProjMat * mViewMat;
}