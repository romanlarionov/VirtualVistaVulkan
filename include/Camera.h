
#ifndef VIRTUALVISTA_CAMERA_H
#define VIRTUALVISTA_CAMERA_H

#include <vector>

#include "Entity.h"
#include "Utils.h"

namespace vv
{
	class Camera : public Entity
	{
	public:
		Camera() = default;
		~Camera() = default;

		/*
		 * Creates a perspective camera.
		 */
        void create(float fov, float near_plane, float far_plane);

		/*
		 *
		 */
		void shutDown();

        /*
         * Returns a vector pointed towards the forward direction of the camera.
         */
        glm::vec3 getForwardDirection() const;

        /*
         * Returns a vector pointed towards the side of the camera. i.e. positive x-axis.
         */
        glm::vec3 getSidewaysDirection() const;

        /*
         *
         */
        virtual void translate(glm::vec3 translation);

        /*
         * Camera requires special rotation mechanics.
         * This perspective camera only allows rotations around the camera's relative coordinate frame in terms of pitch and yaw
         */
        void rotate(float yaw, float pitch);

        /*
         * Used for updating descriptor data during main loop.
         */
        glm::mat4 getProjectionMatrix(float aspect) const;
        glm::mat4 getViewMatrix() const;
		
	private:
        float m_fov_y;
        float m_near_plane;
        float m_far_plane;

        glm::mat4 m_projection_mat;
        glm::mat4 m_view_mat;

        glm::vec3 m_look_at_point;
        glm::vec3 m_up_vec;

        virtual void rotate(float angle, glm::vec3 axis) {};
	};
}

#endif // VIRTUALVISTA_CAMERA_H