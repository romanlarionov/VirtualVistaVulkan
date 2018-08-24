
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
        m_vertices = vertices;
        m_indices = indices;
        m_name = name;
        this->material_id = material_id;

        m_vertex_buffer.create(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(m_vertices[0]) * m_vertices.size());
		m_vertex_buffer.updateAndTransfer(m_vertices.data());

		m_index_buffer.create(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(m_indices[0]) * m_indices.size());
		m_index_buffer.updateAndTransfer(m_indices.data());
	}


	void Mesh::shutDown()
	{
        m_vertex_buffer.shutDown();
        m_index_buffer.shutDown();
        m_vertices.clear();
        m_indices.clear();
	}


    void Mesh::bindBuffers(VkCommandBuffer command_buffer)
    {
        std::array<VkDeviceSize, 1> offsets = { 0 };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_vertex_buffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(command_buffer, m_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }


    void Mesh::render(VkCommandBuffer command_buffer)
    {
        vkCmdDrawIndexed(command_buffer, (uint32_t)m_indices.size(), 1, 0, 0, 0);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}