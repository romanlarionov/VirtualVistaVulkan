
#ifndef VIRTUALVISTA_VULKANSAMPLER_H
#define VIRTUALVISTA_VULKANSAMPLER_H

#include "VulkanDevice.h"

namespace vv
{
	class VulkanSampler
	{
	public:
        VkSampler sampler;
        VkFilter mag_filter;
        VkFilter min_filter;
        VkSamplerAddressMode u_address_mode;
        VkSamplerAddressMode v_address_mode;
        VkSamplerAddressMode w_address_mode;
        bool uses_anisotropy;
        float max_anisotropy;
        VkSamplerMipmapMode mipmap_mode;
        float mip_lod_bias;
        float min_lod;
        float max_lod;
        bool uses_unnormalized_coordinates;

		VulkanSampler();
		~VulkanSampler();

		/*
		 * Creates a sampler based on the passed parameters. Performs checks on input to check if it's valid.
		 */
        void create(VulkanDevice *device, VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode u, VkSamplerAddressMode v,
                    VkSamplerAddressMode w, bool enable_anisotropy, float max_anisotropy, VkSamplerMipmapMode mipmap_mode,
                    float mip_lod_bias, float min_lod, float max_lod, bool use_unnormalized_coordinates);

		/*
		 *
		 */
		void shutDown();

    private:
		VulkanDevice *_device;
	};
}

#endif // VIRTUALVISTA_VULKANSAMPLER_H