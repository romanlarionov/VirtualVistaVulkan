
#include "VulkanSampler.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanSampler::VulkanSampler()
	{
	}


	VulkanSampler::~VulkanSampler()
	{
	}


    void VulkanSampler::create(VulkanDevice *device, VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode u,
                               VkSamplerAddressMode v, VkSamplerAddressMode w, bool enable_anisotropy, float max_anisotropy,
                               VkSamplerMipmapMode mipmap_mode, float mip_lod_bias, float min_lod, float max_lod, bool use_unnormalized_coordinates)
    {
        VV_ASSERT(max_anisotropy <= 16, "VulkanSampler cannot have anisotropy higher than 16");
        if (use_unnormalized_coordinates)
        {
            VV_ASSERT(mag_filter == min_filter, "VulkanSampler must have mag and min filter equal when using unnormalized coordinates");
            VV_ASSERT(mipmap_mode == VK_SAMPLER_MIPMAP_MODE_NEAREST, "VulkanSampler must use VK_SAMPLER_MIPMAP_MODE_NEAREST when using unnormalized coordinates");
            VV_ASSERT(min_lod == 0 && max_lod == 0, "VulkanSampler's min and max lod must be 0 when using unnormalized coordinates");
            VV_ASSERT(u == v, "VulkanSampler's u and v addressing modes must be equal when using unnormalized coordinates");
            VV_ASSERT(u == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE || u == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                "VulkanSampler's address modes must have must use either VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER when using unnormalized coordinates");
            VV_ASSERT(!enable_anisotropy, "VulkanSampler doesn't support anisotropy when using unnormalized coordinates");
        }

        m_device = device;
        this->mag_filter = mag_filter;
        this->min_filter = min_filter;
        this->u_address_mode = u;
        this->v_address_mode = v;
        this->w_address_mode = w;
        this->uses_anisotropy = enable_anisotropy;
        this->max_anisotropy = max_anisotropy;
        this->mipmap_mode = mipmap_mode;
        this->mip_lod_bias = mip_lod_bias;
        this->min_lod = min_lod;
        this->max_lod = max_lod;
        this->uses_unnormalized_coordinates = use_unnormalized_coordinates;

        VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = mag_filter;
		sampler_create_info.minFilter = min_filter;

		// can be a number of things: https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkSamplerAddressMode
        sampler_create_info.addressModeU = u;
		sampler_create_info.addressModeV = v;
		sampler_create_info.addressModeW = w;

		sampler_create_info.anisotropyEnable = enable_anisotropy;
		sampler_create_info.maxAnisotropy = max_anisotropy;

		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // black, white, transparent
		sampler_create_info.unnormalizedCoordinates = use_unnormalized_coordinates;

		sampler_create_info.mipmapMode = mipmap_mode;
		sampler_create_info.mipLodBias = mip_lod_bias;
		sampler_create_info.minLod = min_lod;
		sampler_create_info.maxLod = max_lod; // todo: figure out how lod works with these things

		VV_CHECK_SUCCESS(vkCreateSampler(m_device->logical_device, &sampler_create_info, nullptr, &sampler));
    }


	void VulkanSampler::shutDown()
	{
        vkDestroySampler(m_device->logical_device, sampler, nullptr);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}