
#ifndef VIRTUALVISTA_VULKANIMAGE_H
#define VIRTUALVISTA_VULKANIMAGE_H

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include "Utils.h"
#include "VulkanDevice.h"

namespace vv
{
    struct FormatInfo
    {
        uint32_t block_size;
        VkExtent3D block_extent;
    };

	class VulkanImage
	{
	public:
		VkImage image;
		VkFormat format;
		VkImageAspectFlags aspect_flags;
        VkImageType type;
        uint32_t mip_levels;
        uint32_t array_layers;
        VkSampleCountFlagBits sample_count;
        VkImageLayout initial_layout;
        int width;
		int height;
		int depth;

		VulkanImage();
		~VulkanImage();

        /*
         * Allocates device memory for an image buffer with the given specifications.
         */
        void create(VulkanDevice *device, VkExtent3D extent, VkFormat format, VkImageType type, VkImageCreateFlags flags,
                    VkImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t array_layers,
                    VkImageLayout initial_layout, VkSampleCountFlagBits sample_count);

        /*
		 * Creates an image from existing image. Mainly for swap chain image support.
		 */
	    void createFromImage(VulkanDevice *device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags,
                             uint32_t mip_levels, uint32_t array_layers);

        /*
         * Creates a special image intended to be used as a depth/stencil attachment to a framebuffer.
         */
        void createDepthAttachment(VulkanDevice *device, VkExtent2D extent, VkImageTiling tiling, VkFormatFeatureFlags features);

		/*
		 * Removes allocated device memory.
		 */
		void shutDown();
	
        /*
         * Performs update and transfer operation in single step.
         */
        void updateAndTransfer(void *data, VkDeviceSize size_in_bytes);

		/*
		 * Returns whether this image format supports stencil operations.
		 */
		bool hasStencilComponent();

	private:
		VulkanDevice *m_device;
		VkImage m_staging_image			= VK_NULL_HANDLE;
        VkBuffer m_staging_buffer       = VK_NULL_HANDLE;
		VkDeviceMemory m_staging_memory	= VK_NULL_HANDLE;
		VkDeviceMemory m_image_memory	= VK_NULL_HANDLE;

        std::unordered_map<VkFormat, FormatInfo> m_format_info_table =
        {
              { VK_FORMAT_R8G8B8A8_UNORM, { 4, { 1, 1, 1 } } }
            , { VK_FORMAT_R32G32_SFLOAT, { 8, { 1, 1, 1 } } }
            , { VK_FORMAT_R32G32B32A32_SFLOAT, { 16, { 1, 1, 1 } } }
            , { VK_FORMAT_BC3_UNORM_BLOCK, { 16, { 4, 4, 1 } } }
            , { VK_FORMAT_R8_UNORM, { 1, { 1, 1, 1 } } }
            , { VK_FORMAT_R8G8B8_UNORM, { 3, { 1, 1, 1 } } }
        };

        std::unordered_map<VkPipelineStageFlags, VkQueueFlags> m_pipeline_stage_queue_support_lut =
        {
              { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT}
            , { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT }
            , { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_QUEUE_COMPUTE_BIT }
            , { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT }
            , { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT }
            , { VK_PIPELINE_STAGE_HOST_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT }
            , { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT }
            , { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT }
        };

        /*
         * Allocates a VkBuffer to use during transfer operations between host and device memory.
         */
        void allocateTransferMemory(VkDeviceSize size_in_bytes);

		/*
		 * Creates the Vulkan abstraction for a data buffer with the given specifications.
		 */
        void allocateMemory(VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageLayout initial_layout,
                            VkSampleCountFlagBits sample_count, VkMemoryPropertyFlags memory_properties, VkImage &image,
                            VkDeviceMemory &memory);
	
		/*
		 * Move the linearly stored staging image into an optimal texture storage layout.
		 * https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkImageLayout
		 */
		void transformImageLayout(VkImage image, VkImageSubresourceRange subresource_range, VkImageLayout old_layout, VkImageLayout new_layout);

        void transformImageLayout(VkCommandBuffer command_buffer, VkImage image, VkImageSubresourceRange subresource_range, VkImageLayout old_layout,
                                  VkImageLayout new_layout);

        /*
         * Determines the correct pair of src + dst memory access constraints to prepare for during image layout transformation.
         */
        VkImageMemoryBarrier determineAccessMasks(VkImage image, VkImageSubresourceRange subresource_range,
                                                  VkImageLayout old_layout, VkImageLayout new_layout);
	};
}

#endif // VIRTUALVISTA_VULKANIMAGE_H