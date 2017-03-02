
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

        _vertex_buffer.create(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(_vertices[0]) * _vertices.size());
		_vertex_buffer.updateAndTransfer(_vertices.data());

		_index_buffer.create(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(_indices[0]) * _indices.size());
		_index_buffer.updateAndTransfer(_indices.data());
	}


	void Mesh::shutDown()
	{
        _vertex_buffer.shutDown();
        _index_buffer.shutDown();
	}


    void Mesh::bindBuffers(VkCommandBuffer command_buffer)
    {
        std::array<VkDeviceSize, 1> offsets = { 0 };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &_vertex_buffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(command_buffer, _index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }


    void Mesh::render(VkCommandBuffer command_buffer)
    {
        vkCmdDrawIndexed(command_buffer, (uint32_t)_indices.size(), 1, 0, 0, 0);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}