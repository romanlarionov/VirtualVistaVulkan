
#include "Camera.h"
#include "Settings.h"

#include "glm/gtx/rotate_vector.hpp"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Camera::Camera()
	{
        _is_visible = false;
        _is_renderable = false;
	}


	Camera::~Camera()
	{
	}


    void Camera::create(float fov, float near_plane, float far_plane)
	{
        VV_ASSERT(fov > 0, "ERROR: fov has to be positive");
        VV_ASSERT(near_plane > 0, "ERROR: near plane has to be positive");

        _fov_y = fov;
        _near_plane = near_plane;
        _far_plane = far_plane;
        _look_at_point = glm::vec3(0.0f, 0.0f, -1.0f); // todo: maybe don't hardcode these?
        _up_vec = glm::vec3(0.0f, 1.0f, 0.0f);
	}


	void Camera::shutDown()
	{
	}


    glm::vec3 Camera::getForwardDirection() const
    {
        return glm::normalize(_look_at_point - Entity::getPosition());
    }


    glm::vec3 Camera::getSidewaysDirection() const
    {
        return glm::normalize(glm::cross(getForwardDirection(), _up_vec));
    }


    void Camera::translate(glm::vec3 translation)
    {
        Entity::translate(translation);
        _look_at_point += translation;
    }


    void Camera::rotate(float yaw, float pitch)
    {
        // construct coordinate frame around the camera center
        auto w = getForwardDirection();
        auto u = getSidewaysDirection();
        auto v = glm::normalize(glm::cross(u, w));

        pitch = (pitch > 89.0f) ? 89.0f : pitch;
        pitch = (pitch < -89.0f) ? -89.0f : pitch;
        auto position = Entity::getPosition();

        // rotate look at point around relative frame's axes
        _look_at_point = glm::rotate(_look_at_point - position, pitch, u) + position;
        _look_at_point = glm::rotate(_look_at_point - position, -yaw, v) + position;
    }


    glm::mat4 Camera::getProjectionMatrix(float aspect) const
    {
        return glm::perspective(_fov_y, aspect, _near_plane, _far_plane);
    }


    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAt(Entity::getPosition(), _look_at_point, _up_vec);
    }

	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
