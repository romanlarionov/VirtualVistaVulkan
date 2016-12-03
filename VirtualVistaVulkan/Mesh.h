
#ifndef VIRTUALVISTA_MESH_H
#define VIRTUALVISTA_MESH_H

#include <vector>
#include <string>

#include "VulkanBuffer.h"

namespace vv
{
	class Mesh
	{
	public:

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// todo: prob don't want to have per mesh buffers. Important optimization is to batch together all geometry to single buffer prior to rendering.
		VulkanBuffer vertex_buffer;  
		VulkanBuffer index_buffer;

		Mesh();
		~Mesh();

		/*
		 * Creates an object with all geometric and material properties needed for rendering.
		 * todo: replace with proper model loading class. need to detach ability of loading assets from the actual entity itself.
		 * note: can load different types of 3d models. ASSUMING ONLY GLTF TYPES FOR NOW!!
		 */
		void init(std::string filename);

		/*
		 * 
		 */
		void shutDown();

	private:

		void loadOBJ(std::string filename);
		void loadGLTF(std::string filename);

	};
}

#endif // VIRTUALVISTA_MESH_H