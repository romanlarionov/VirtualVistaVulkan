
#ifndef VIRTUALVISTA_MATERIAL_H
#define VIRTUALVISTA_MATERIAL_H

#include <vector>

#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanImageView.h"
#include "MaterialTemplate.h"

namespace vv
{
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
         *
         */
        void addUniformBuffer(VulkanBuffer *texture, int binding);

        /*
         *
         */
        void addTexture(VulkanImage *texture, int binding, VkSampler sampler);

        /*
         *
         */
        void updateDescriptorSets() const;

        /*
         *
         */
        void bindDescriptorSets(VkCommandBuffer command_buffer, std::vector<VkDescriptorSet> descriptor_sets) const;

	private:
        VulkanDevice *_device;
        std::vector<VkWriteDescriptorSet> _write_sets;
        VkDescriptorSet _descriptor_set;

        std::vector<std::pair<VkDescriptorBufferInfo, VulkanBuffer *> > _uniform_buffers;
        std::vector<std::pair<VkDescriptorImageInfo, VulkanImageView *> > _textures;
	};
}

#endif // VIRTUALVISTA_MATERIAL_H