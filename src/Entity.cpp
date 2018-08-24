
#include "Entity.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Entity::Entity() :
        m_pose(glm::mat4())
	{
	}


	Entity::~Entity()
	{
	}


	glm::vec3 Entity::getPosition() const
	{
        return glm::vec3(m_pose[3]);
	}


    glm::mat3 Entity::getRotation() const
	{
        return glm::mat3(m_pose);
	}


    void Entity::translate(glm::vec3 translation)
	{
        m_pose = glm::translate(m_pose, translation);
	}


    void Entity::rotate(float angle, glm::vec3 axis)
	{
        m_pose = glm::rotate(m_pose, angle, axis);
	}


    void Entity::scale(glm::vec3 scaling)
	{
        m_pose = glm::scale(m_pose, scaling);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
