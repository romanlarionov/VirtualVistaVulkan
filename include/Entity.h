
#ifndef VIRTUALVISTA_ENTITY_H
#define VIRTUALVISTA_ENTITY_H

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace vv
{
	class Entity
	{
	public:
		Entity();
		~Entity();

        glm::vec3 getPosition() const;
        glm::mat3 getRotation() const;

        virtual void translate(glm::vec3 translation);
        virtual void rotate(float angle, glm::vec3 axis);
        void scale(glm::vec3 scaling);

    protected:
        // todo: use for occlusion checking
        bool m_is_visible    = false;
        bool m_is_renderable = false;

        glm::mat4 m_pose;

	private:

	};
}

#endif // VIRTUALVISTA_ENTITY_H