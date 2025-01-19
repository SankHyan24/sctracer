#pragma once
#include <utils/mathUtils.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace scTracer::Core {

    class CameraBase { // interface
    public:
        virtual glm::mat4 getViewMatrix() = 0;
        virtual glm::mat4 getProjectionMatrix() = 0;
        virtual glm::mat4 getVP() = 0;
    };


    class Camera : CameraBase {
    public:
        Camera(glm::vec3 position, glm::vec3 lookAt, float fovDegree);

        virtual glm::mat4 getViewMatrix() override {
            return mViewMat;
        }

        virtual glm::mat4 getProjectionMatrix() override {
            return mProjMat;
        }

        virtual glm::mat4 getVP() override {
            return mVPMat;
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

