
#ifndef VIRTUALVISTA_CAMERA_H
#define VIRTUALVISTA_CAMERA_H

#include <vector>

#include "Entity.h"

namespace vv
{
	class Camera : public Entity
	{
	public:
		Camera();
		~Camera();

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
        float _fov_y;
        float _near_plane;
        float _far_plane;

        glm::mat4 _projection_mat;
        glm::mat4 _view_mat;

        glm::vec3 _look_at_point;
        glm::vec3 _up_vec;

        virtual void rotate(float angle, glm::vec3 axis) {};
	};
}

#endif // VIRTUALVISTA_CAMERA_H