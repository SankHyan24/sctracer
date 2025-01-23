#pragma once
#include <iostream>
#include <utils/mathUtils.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace scTracer::Core {

    class CameraBase { // interface
    public:
        CameraBase() = default;
        virtual ~CameraBase() = default;
        virtual glm::mat4 getViewMatrix() = 0;
        virtual glm::mat4 getProjectionMatrix() = 0;
        virtual glm::mat4 getVP() = 0;
        virtual void printDebugInfo() = 0;
    };


    class Camera : CameraBase {
    public:
        Camera(glm::vec3 position, glm::vec3 lookAt, float fovDegree);
        Camera(glm::mat4 transform, float fovDegree);
        virtual ~Camera() = default;

        virtual glm::mat4 getViewMatrix() override {
            return mViewMat;
        }

        virtual glm::mat4 getProjectionMatrix() override {
            return mProjMat;
        }

        virtual glm::mat4 getVP() override {
            return mVPMat;
        }

        virtual void printDebugInfo() override {
            std::cout << "Camera Debug Info:" << std::endl;
            std::cout << "Position: " << mPosition.x << " " << mPosition.y << " " << mPosition.z << std::endl;
            std::cout << "Front: " << mFront.x << " " << mFront.y << " " << mFront.z << std::endl;
            std::cout << "Up: " << mUp.x << " " << mUp.y << " " << mUp.z << std::endl;
            std::cout << "Right: " << mRight.x << " " << mRight.y << " " << mRight.z << std::endl;
            std::cout << "Focal Distance: " << mFocalDist << std::endl;
            std::cout << "Aperture: " << mAperture << std::endl;
            std::cout << "FOV: " << mFov << std::endl;
        }

        glm::vec3 mPosition;
        glm::vec3 mFront;
        glm::vec3 mUp;
        glm::vec3 mRight;

        float mFocalDist;
        float mAperture;
        float mFov;

    private:
        void updateCameraVectors();
        glm::vec3  worldUp;
        glm::vec3  pivot;

        float pitch;
        float radius;
        float yaw;

        glm::mat4 mViewMat;
        glm::mat4 mProjMat;
        glm::mat4 mVPMat;
    };
}

