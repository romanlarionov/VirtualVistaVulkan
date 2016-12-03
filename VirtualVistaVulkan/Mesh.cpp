
//#define TINYGLTF_LOADER_IMPLEMENTATION
//#include "tiny_gltf_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <set>

#include "Mesh.h"
#include "Utils.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Mesh::Mesh()
	{
	}


	Mesh::~Mesh()
	{
	}


	void Mesh::init(std::string filename)
	{
		loadOBJ(filename);
	}


	void Mesh::shutDown()
	{

	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
	void Mesh::loadOBJ(std::string filename)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		VV_ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str()), "Model: " + filename + " not loaded correctly");

		std::unordered_map<Vertex, int> vertex_map;

		// Convert all submeshes to single model
		// todo: hack solution. assumes only single material.
		// todo: currently not loading texture with geometry.
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex = {};

				vertex.color = glm::vec3(1.0, 1.0, 1.0);
				vertex.position = glm::vec3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				);

				vertex.texCoord = glm::vec2(
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				);

				if (vertex_map.count(vertex) == 0)
				{
					vertex_map[vertex] = vertices.size();
					vertices.push_back(vertex);
				}

				indices.push_back(vertex_map[vertex]);
			}
		}
	}


	void Mesh::loadGLTF(std::string filename)
	{

	}

}