
#ifndef VIRTUALVISTA_MESH_H
#define VIRTUALVISTA_MESH_H

#include <vector>
#include <string>

#include "VulkanDevice.h"
#include "VulkanBuffer.h"

namespace vv
{
	class Mesh
	{
	public:
        int material_id;

		Mesh();
		~Mesh();

		/*
		 * Stores all geometry information for a submesh within a model hierarchy.
         * Called from Model wrapper class. Should not be called outside of this context.
		 */
		void create(VulkanDevice *device, std::string name, std::vector<Vertex> vertices, std::vector<uint32_t> indices, int material_id);

		/*
		 * 
		 */
		void shutDown();

        /*
         * Binds all geometry data in preparation for rendering.
         */
        void bindBuffers(VkCommandBuffer command_buffer);


        /*
         * Per mesh rendering using private vertex + index vulkan buffers.
         */
        void render(VkCommandBuffer command_buffer);

	private:
        std::string m_name;

		VulkanBuffer m_vertex_buffer;
		VulkanBuffer m_index_buffer;

		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

	};
}

#endif // VIRTUALVISTA_MESH_H