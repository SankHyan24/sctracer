#include <core/camera.hpp>
using namespace scTracer::Core;

Camera::Camera(glm::vec3 position, glm::vec3 lookAt, float fovDegree, glm::vec3 up )
{
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

Camera::Camera(glm::mat4 transform, float fovDegree)
{
    mPosition = glm::vec3(transform[3]);
    mPosition = glm::vec3(0, 1, 6.8);
    mFront = glm::normalize(glm::vec3(transform[2]));
    // mFront = glm::normalize(glm::vec3(1, 0, 0));
    worldUp = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    pitch = Utils::mathUtils::radians2degrees(asin(mFront.y));
    yaw = Utils::mathUtils::radians2degrees(atan2(mFront.z, mFront.x));
    radius = glm::length(mPosition);

    mFov = Utils::mathUtils::degrees2radians(fovDegree);
    mFocalDist = 0.1f;
    mAperture = 0.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    mFront = glm::normalize(mFront);
    mRight = glm::normalize(glm::cross(mFront, worldUp));
    mUp = glm::normalize(glm::cross(mRight, mFront));

    mViewMat = glm::lookAt(mPosition, mPosition + mFront, mUp);
    mProjMat = glm::perspective(mFov, 1.0f, 0.1f, 100.0f);
    mVPMat = mProjMat * mViewMat;
}