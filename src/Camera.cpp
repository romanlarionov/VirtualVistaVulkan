
#include "Camera.h"
#include "Settings.h"

#include "glm/gtx/rotate_vector.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
    void Camera::create(float fov, float near_plane, float far_plane)
	{
        VV_ASSERT(fov > 0, "ERROR: fov has to be positive");
        VV_ASSERT(near_plane > 0, "ERROR: near plane has to be positive");

        m_fov_y = fov;
        m_near_plane = near_plane;
        m_far_plane = far_plane;
        m_look_at_point = glm::vec3(0.0f, 0.0f, 1.0f);
        m_up_vec = glm::vec3(0.0f, 1.0f, 0.0f);
	}


	void Camera::shutDown()
	{
	}


    glm::vec3 Camera::getForwardDirection() const
    {
        return glm::normalize(m_look_at_point - Entity::getPosition());
    }


    glm::vec3 Camera::getSidewaysDirection() const
    {
        return glm::normalize(glm::cross(getForwardDirection(), m_up_vec));
    }


    void Camera::translate(glm::vec3 translation)
    {
        Entity::translate(translation);
        m_look_at_point += translation;
    }


    void Camera::rotate(float yaw, float pitch)
    {
        // construct coordinate frame around the camera center
        auto w = getForwardDirection();
        auto u = getSidewaysDirection();
        auto v = glm::normalize(glm::cross(u, w));

        pitch = (pitch > 83.0f) ? 83.0f : pitch;
        pitch = (pitch < -83.0f) ? -83.0f : pitch;
        auto position = Entity::getPosition();

        // find component-wise rotation quaternions
        glm::quat pitch_quat = glm::angleAxis(-pitch, u);
        glm::quat yaw_quat = glm::angleAxis(-yaw, m_up_vec);

        // combine both components
        glm::quat combined_rotations = glm::normalize(glm::cross(pitch_quat, yaw_quat));
        m_look_at_point = glm::rotate(combined_rotations, m_look_at_point - position) + position;
    }


    glm::mat4 Camera::getProjectionMatrix(float aspect) const
    {
        auto mat = glm::perspective(m_fov_y, aspect, m_near_plane, m_far_plane);
        mat[1][1] *= -1.f;
        return mat;
    }


    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAt(Entity::getPosition(), m_look_at_point, m_up_vec);
    }

	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
