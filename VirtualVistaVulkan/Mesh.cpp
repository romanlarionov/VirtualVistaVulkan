
#include "Mesh.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Mesh::Mesh()
	{
	}


	Mesh::~Mesh()
	{
	}


	void Mesh::create(VulkanDevice *device, std::string name, std::vector<Vertex> vertices, std::vector<uint32_t> indices, int material_id)
	{
        _vertices = vertices;
        _indices = indices;
        _name = name;
        this->material_id = material_id;

		vertex_buffer.create(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex), vertices.size());
		vertex_buffer.updateAndTransfer(vertices.data());
		index_buffer.create(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t), indices.size());
		index_buffer.updateAndTransfer(indices.data());
	}


	void Mesh::shutDown()
	{
        vertex_buffer.shutDown();
        index_buffer.shutDown();
	}


    void Mesh::bindBuffers(VkCommandBuffer command_buffer) const
    {
        std::array<VkDeviceSize, 1> offsets = { 0 };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }


    void Mesh::render(VkCommandBuffer command_buffer) const
    {
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}