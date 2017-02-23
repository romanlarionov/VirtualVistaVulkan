
#include "Entity.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Entity::Entity() :
        _pose(glm::mat4())
	{
	}


	Entity::~Entity()
	{
	}


	glm::vec3 Entity::getPosition() const
	{
        return glm::vec3(_pose[3]);
	}


    glm::mat3 Entity::getRotation() const
	{
        return glm::mat3(_pose);
	}


    void Entity::translate(glm::vec3 translation)
	{
        _pose = glm::translate(_pose, translation);
	}


    void Entity::rotate(float angle, glm::vec3 axis)
	{
        _pose = glm::rotate(_pose, angle, axis);
	}


    void Entity::scale(glm::vec3 scaling)
	{
        _pose = glm::scale(_pose, scaling);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
