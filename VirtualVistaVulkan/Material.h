
#ifndef VIRTUALVISTA_MATERIAL_H
#define VIRTUALVISTA_MATERIAL_H

#include <vector>
#include <utility>

#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanImageView.h"
#include "MaterialTemplate.h"

namespace vv
{
    struct UBOStore
    {
        VkDescriptorBufferInfo info;
        VulkanBuffer *buffer;
    };

    struct TextureStore
    {
        VkDescriptorImageInfo info;
        VulkanImageView *view;
    };

	class Material
	{
	public:
        MaterialTemplate *material_template;

		Material();
		~Material();

		/*
		 * This class serves as an instance of a specified MaterialTemplate.
         * It holds the actual data relevant to the creation of a material such as VkDescriptorSets.
         *
         * note: this should only be called from within MaterialTemplate, which manages all such material instances.
		 */
        void create(VulkanDevice *device, MaterialTemplate *material_template, VkDescriptorPool descriptor_pool);

		/*
		 *
		 */
		void shutDown();
		
        /*
         * Instructs this instance to support a uniform buffer binding and maintains ownership over the data.
         */
        void addUniformBuffer(VulkanBuffer *uniform_buffer, int binding);

        /*
         * Instructs this instance to support a texture binding and maintains ownership over the data.
         */
        void addTexture(VulkanImage *texture, int binding, VkSampler sampler);

        /*
         * Updates the contents of the descriptor set with the uniform + samplers provided via addUniformBuffer and addTexture.
         */
        void updateDescriptorSets() const;

        /*
         * Binds all descriptor sets this instance has ownership over. Should be called at render time.
         */
        void bindDescriptorSets(VkCommandBuffer command_buffer) const;

	private:
        VulkanDevice *_device;
        std::vector<VkWriteDescriptorSet> _write_sets;
        VkDescriptorSet _descriptor_set;

        std::vector<UBOStore *> _uniform_buffers;
        std::vector<TextureStore *> _textures;

        std::vector<VulkanImage *> _image_store;
	};
}

#endif // VIRTUALVISTA_MATERIAL_H