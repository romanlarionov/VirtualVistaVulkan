
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
        friend class MaterialTemplate;

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
        void create(MaterialTemplate *material_template);

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
        void addTexture(VulkanImage *texture, int binding);

        /*
         *
         */
        void updateDescriptorSets() const;

        /*
         *
         */
        void bindDescriptorSets(VkCommandBuffer command_buffer, VkDescriptorSet descriptor_set) const;

        /*
         *
         */
        std::vector<DescriptorType> getDescriptorOrdering() const;

	private:
        std::vector<VkWriteDescriptorSet> write_sets_;
        VkDescriptorSet descriptor_set_;

        std::vector<std::pair<VkDescriptorBufferInfo, VulkanBuffer *> > uniform_buffers_;
        std::vector<std::pair<VkDescriptorImageInfo, VulkanImageView *> > textures_;
	};
}

#endif // VIRTUALVISTA_MATERIAL_H