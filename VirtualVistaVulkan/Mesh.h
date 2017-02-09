
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
		// todo: prob don't want to have per mesh buffers. important optimization
        //       is to batch together all geometry to single buffer prior to rendering.
		VulkanBuffer vertex_buffer;
		VulkanBuffer index_buffer;
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
         *
         */
        void bindBuffers(VkCommandBuffer command_buffer) const;


        /*
         *
         */
        void render(VkCommandBuffer command_buffer) const;

	private:
        std::string _name;
		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;

	};
}

#endif // VIRTUALVISTA_MESH_H