
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <chrono>
#include <array>

#include "VulkanRenderer.h"
#include "Settings.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WIN32
#define NOMINMAX  
#include <Windows.h>
#endif

namespace vv
{
	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}


	void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
#ifdef _DEBUG
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
			func(instance, callback, pAllocator);
#endif
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanRenderer::VulkanRenderer()
	{
	}


	VulkanRenderer::~VulkanRenderer()
	{
	}


	void VulkanRenderer::create()
	{
		try
		{
			// Note: this is a very specific order and is not to be messed with.
			createWindow();
			createVulkanInstance();
			window_->createSurface(instance_);

			setupDebugCallback();
			createVulkanDevices();

			swap_chain_ = new VulkanSwapChain();
			swap_chain_->create(physical_devices_[0], window_);

			render_pass_ = new VulkanRenderPass();
			render_pass_->create(physical_devices_[0], swap_chain_);

			createFrameBuffers();

			image_ready_semaphore_ = util::createVulkanSemaphore(this->physical_devices_[0]->logical_device);
			rendering_complete_semaphore_ = util::createVulkanSemaphore(this->physical_devices_[0]->logical_device);

			// todo: all below should be generalized and moved to application file

            // creates shader and pipeline
            std::vector<DescriptorType> descriptor_orderings;
            descriptor_orderings.push_back(DescriptorType::CONSTANTS);

            std::vector<VulkanDescriptorSetLayout> descriptor_set_layouts;
            //model_descriptor_set_layout_.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            model_descriptor_set_layout_.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            model_descriptor_set_layout_.create(physical_devices_[0]);
            descriptor_set_layouts.push_back(model_descriptor_set_layout_);

            general_descriptor_set_layout_.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
            general_descriptor_set_layout_.create(physical_devices_[0]);
            descriptor_set_layouts.push_back(general_descriptor_set_layout_);

			createDescriptorPool();

            ubo_ = { glm::mat4(), glm::mat4(), glm::mat4(), glm::vec3(0.0, 0.0, 1.0) };
			uniform_buffer_ = new VulkanBuffer();
			uniform_buffer_->create(physical_devices_[0], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ubo_), 1);

            createGeneralDescriptorSet();

			createSampler();

			shader_ = new Shader();
			shader_->create(physical_devices_[0], "triangle");

            MaterialTemplate material_template;
            // todo: find workaround for material name issue
            material_template.create(physical_devices_[0], "banner", shader_, descriptor_pool_, descriptor_set_layouts, descriptor_orderings, sampler_, render_pass_);
            material_templates_.push_back(material_template);

            Model model;
            models_.push_back(model);

            asset_manager_ = new AssetManager();
            asset_manager_->create(physical_devices_[0], material_templates_);
            //asset_manager_->loadModel("OBJ/hammardillo/", "hammardillo.obj", models_[0]);
            asset_manager_->loadModel("OBJ/sponza/", "banner.obj", models_[0]);
			
			createCommandBuffers();
		}
		catch (const std::runtime_error& e)
		{
			throw e;
		}
	}


	void VulkanRenderer::shutDown()
	{
		// accounts for the issue of a logical device that might be executing commands when a terminating command is issued.
		vkDeviceWaitIdle(physical_devices_[0]->logical_device);

        for (auto &t : material_templates_)
            t.shutDown();

        for (auto &m : models_)
            m.shutDown();

		uniform_buffer_->shutDown(); delete uniform_buffer_;

	  	// Async devices
	  	vkDestroySemaphore(physical_devices_[0]->logical_device, image_ready_semaphore_, nullptr);
	  	vkDestroySemaphore(physical_devices_[0]->logical_device, rendering_complete_semaphore_, nullptr);

	  	vkDestroyDescriptorPool(physical_devices_[0]->logical_device, descriptor_pool_, nullptr);
        general_descriptor_set_layout_.shutDown();
      
        vkDestroySampler(physical_devices_[0]->logical_device, sampler_, nullptr);

	  	// Graphics Pipeline
	  	shader_->shutDown(); delete shader_;
	  	render_pass_->shutDown(); delete render_pass_;

	  	for (std::size_t j = 0; j < frame_buffers_.size(); ++j)
	  		vkDestroyFramebuffer(physical_devices_[0]->logical_device, frame_buffers_[j], nullptr);

	  	swap_chain_->shutDown(physical_devices_[0]); delete swap_chain_;

	  	// Physical/Logical devices
	  	physical_devices_[0]->shutDown(); delete physical_devices_[0];
        window_->shutDown(instance_); delete window_;
		destroyDebugReportCallbackEXT(instance_, debug_callback_, nullptr);
		vkDestroyInstance(instance_, nullptr);
	}


	void VulkanRenderer::run()
	{
		// Poll window specific updates and input.
		window_->run();

		// Update uniforms
		// todo: remove
		static auto start_time = std::chrono::high_resolution_clock::now();
		auto curr_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - start_time).count() / 1000.0f;

		ubo_.model = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		ubo_.model = glm::rotate(ubo_.model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo_.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo_.proj = glm::perspective(glm::radians(45.0f), swap_chain_->extent.width / (float) swap_chain_->extent.height, 0.1f, 10.0f);
		ubo_.normal = glm::vec3(glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		uniform_buffer_->updateAndTransfer(&ubo_);

		// Draw Frame
		/// Acquire an image from the swap chain
		uint32_t image_index = 0;
		swap_chain_->acquireNextImage(physical_devices_[0], image_ready_semaphore_, image_index);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		/// tell the queue to wait until a command buffer successfully attaches a swap chain image as a color attachment (wait until its ready to begin rendering).
		std::array<VkPipelineStageFlags, 1> wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_ready_semaphore_;
		submit_info.pWaitDstStageMask = wait_stages.data();

		/// Set the command buffer that will be used to rendering to be the one we waited for.
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers_[image_index];

		/// Detail the semaphore that marks when rendering is complete.
		std::array<VkSemaphore, 1> signal_semaphores = { rendering_complete_semaphore_ };
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores.data();

		VV_CHECK_SUCCESS(vkQueueSubmit(physical_devices_[0]->graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
		swap_chain_->queuePresent(physical_devices_[0]->graphics_queue, image_index, rendering_complete_semaphore_);
	}


	bool VulkanRenderer::shouldStop()
	{
		// todo: add other conditions
		return window_->shouldClose();
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
	void VulkanRenderer::createWindow()
	{
		window_ = new GLFWWindow;
		window_->create();
	}


	void VulkanRenderer::createVulkanInstance()
	{
		VV_ASSERT(checkValidationLayerSupport(), "Validation layers requested are not available on this system.");

		// Instance Creation
		std::string application_name = Settings::inst()->getApplicationName();
		std::string engine_name = Settings::inst()->getEngineName();

		// todo: offload the version stuff to settings
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = application_name.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = engine_name.c_str();
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		// Ensure required extensions are found.
		VV_ASSERT(checkInstanceExtensionSupport(), "Extensions requested, but are not available on this system.");

		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &app_info;

		auto required_extensions = getRequiredExtensions();
		instance_create_info.enabledExtensionCount = (uint32_t)required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();

#ifdef _DEBUG
		instance_create_info.enabledLayerCount = (uint32_t)used_validation_layers_.size();
		instance_create_info.ppEnabledLayerNames = used_validation_layers_.data();
#else
		instance_create_info.enabledLayerCount = 0;
#endif

		VV_CHECK_SUCCESS(vkCreateInstance(&instance_create_info, nullptr, &instance_));
	}


	std::vector<const char*> VulkanRenderer::getRequiredExtensions()
	{
		std::vector<const char*> extensions;

		// GLFW specific
		for (uint32_t i = 0; i < window_->glfw_extension_count; i++)
			extensions.push_back(window_->glfw_extensions[i]);

		// System wide required (hardcoded)
		for (auto extension : used_instance_extensions_)
			extensions.push_back(extension);

		return extensions;
	}


	bool VulkanRenderer::checkInstanceExtensionSupport()
	{
		std::vector<const char*> required_extensions = getRequiredExtensions();

		uint32_t extension_count = 0;
		VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data()));

		// Compare found extensions with requested ones.
		for (const auto& extension : required_extensions)
		{
			bool extension_found = false;
			for (const auto& found_extension : available_extensions)
				if (strcmp(extension, found_extension.extensionName) == 0)
				{
					extension_found = true;
					break;
				}

			if (!extension_found) return false;
		}

		return true;
	}


	bool VulkanRenderer::checkValidationLayerSupport()
	{
		uint32_t layer_count = 0;
		VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
		std::vector<VkLayerProperties> available_layers(layer_count);
		VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()));

		// Compare found layers with requested ones.
		for (const char* layer : used_validation_layers_)
		{
			bool layer_found = false;
			for (const auto& found_layer : available_layers)
				if (strcmp(layer, found_layer.layerName) == 0)
				{
					layer_found = true;
					break;
				}

			if (!layer_found) return false;
		}

		return true;
	}


	void VulkanRenderer::createVulkanSurface()
	{
		window_->createSurface(instance_);
	}


	/*
	 * Callback that performs all printing of validation layer info.
	 */
	VKAPI_ATTR VkBool32 VKAPI_CALL
		vulkanDebugCallback(VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT obj_type, // object that caused the error
			uint64_t src_obj,
			size_t location,
			int32_t msg_code,
			const char *layer_prefix,
			const char *msg,
			void *usr_data)
	{
		// todo: maybe add logging to files
		// todo: think of other things that might be useful to log
		std::ostringstream stream;
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			stream << "WARNING: ";
		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			stream << "PERFORMANCE: ";
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			stream << "ERROR: ";
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			stream << "DEBUG: ";

		stream << "@[" << layer_prefix << "]" << std::endl;
		stream << msg << std::endl;
		std::cout << stream.str() << std::endl;

#ifdef _WIN32
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			MessageBox(NULL, stream.str().c_str(), "VirtualVista Vulkan Error", 0);
#endif

		return false;
	}


	void VulkanRenderer::setupDebugCallback()
	{
#ifdef _DEBUG

		VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
		debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_callback_create_info.pfnCallback = vulkanDebugCallback;
		debug_callback_create_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;

		VV_CHECK_SUCCESS(createDebugReportCallbackEXT(instance_, &debug_callback_create_info, nullptr, &debug_callback_));
#endif
	}


	void VulkanRenderer::createVulkanDevices()
	{
		// todo: implement ranking system to choose most optimal GPU or order them in increasing order of relevance.
		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr);

		VV_ASSERT(physical_device_count != 0, "Vulkan Error: no gpu with Vulkan support found");

		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());

		// Find any physical devices that might be suitable for on screen rendering.
		for (const auto& device : physical_devices)
		{
			VulkanDevice *vulkan_device = new VulkanDevice;
			vulkan_device->create(device);
			VulkanSurfaceDetailsHandle surface_details_handle = {};
			if (vulkan_device->isSuitable(window_->surface, surface_details_handle))
			{
				vulkan_device->createLogicalDevice(true, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
				physical_devices_.push_back(vulkan_device);
				window_->surface_settings[vulkan_device] = surface_details_handle; // todo: find a better hash code than a pointer
				break; // todo: remove. for now only adding one device.
			}
			else
			{
				vulkan_device->shutDown();
				delete vulkan_device;
			}
		}

		VV_ASSERT(!physical_devices_.empty(), "Vulkan Error: no gpu with Vulkan support found");
	}


     void VulkanRenderer::createDescriptorPool()
	{
        // global pool
        std::vector<VkDescriptorPoolSize> pool_sizes;
        pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_UNIFORM_BUFFERS });
        pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_COMBINED_IMAGE_SAMPLERS });

		VkDescriptorPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.poolSizeCount = (uint32_t)pool_sizes.size();
		create_info.pPoolSizes = pool_sizes.data();
		create_info.maxSets = MAX_DESCRIPTOR_SETS;
		create_info.flags = 0; // can be: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT

        // set up global descriptor pool
		VV_CHECK_SUCCESS(vkCreateDescriptorPool(physical_devices_[0]->logical_device, &create_info, nullptr, &descriptor_pool_));
	}

    
    void VulkanRenderer::createGeneralDescriptorSet()
	{
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = descriptor_pool_;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &general_descriptor_set_layout_.layout;

		VV_CHECK_SUCCESS(vkAllocateDescriptorSets(physical_devices_[0]->logical_device, &alloc_info, &general_descriptor_set_));

		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = uniform_buffer_->buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet write_set = {};
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = general_descriptor_set_;
		write_set.dstBinding = 0;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_set.descriptorCount = 1; // how many elements to update
		write_set.pBufferInfo = &buffer_info;

		vkUpdateDescriptorSets(physical_devices_[0]->logical_device, 1, &write_set, 0, nullptr);
	}


	void VulkanRenderer::createSampler()
	{
		VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST
		sampler_create_info.minFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST

		// can be a number of things: https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkSamplerAddressMode
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		sampler_create_info.anisotropyEnable = VK_TRUE;
		sampler_create_info.maxAnisotropy = 16; // max value

		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // black, white, transparent
		sampler_create_info.unnormalizedCoordinates = VK_FALSE; // [0,1]
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f; // todo: figure out how lod works with these things

		VV_CHECK_SUCCESS(vkCreateSampler(physical_devices_[0]->logical_device, &sampler_create_info, nullptr, &sampler_));
	}


	void VulkanRenderer::createFrameBuffers()
	{
		frame_buffers_.resize(swap_chain_->color_image_views.size());

		for (std::size_t i = 0; i < swap_chain_->color_image_views.size(); ++i)
		{
			std::vector<VkImageView> attachments = { swap_chain_->color_image_views[i]->image_view, swap_chain_->depth_image_view->image_view };

			VkFramebufferCreateInfo frame_buffer_create_info = {};
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.flags = 0;
			frame_buffer_create_info.renderPass = render_pass_->render_pass;
			frame_buffer_create_info.attachmentCount = (uint32_t)attachments.size();
			frame_buffer_create_info.pAttachments = attachments.data();
			frame_buffer_create_info.width = swap_chain_->extent.width;
			frame_buffer_create_info.height = swap_chain_->extent.height;
			frame_buffer_create_info.layers = 1;

			VV_CHECK_SUCCESS(vkCreateFramebuffer(physical_devices_[0]->logical_device, &frame_buffer_create_info, nullptr, &frame_buffers_[i]));
		}
	}

	
	void VulkanRenderer::createCommandBuffers()
	{
        command_buffers_.resize(frame_buffers_.size());

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = physical_devices_[0]->command_pools["graphics"];

        // primary can be sent to pool for execution, but cant be called from other buffers. secondary cant be sent to pool, but can be called from other buffers.
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
        command_buffer_allocate_info.commandBufferCount = (uint32_t)command_buffers_.size();

        VV_CHECK_SUCCESS(vkAllocateCommandBuffers(physical_devices_[0]->logical_device, &command_buffer_allocate_info, command_buffers_.data()));

        std::vector<VkClearValue> clear_values; // todo: offload to settings
        VkClearValue color_value, depth_value;
        color_value.color = { 0.3f, 0.5f, 0.5f, 1.0f };
        depth_value.depthStencil = {1.0f, 0};
        clear_values.push_back(color_value);
        clear_values.push_back(depth_value);

		for (std::size_t i = 0; i < command_buffers_.size(); ++i)
		{
            VkCommandBufferBeginInfo command_buffer_begin_info = {};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // <- tells how long this buffer will be executed
            command_buffer_begin_info.pInheritanceInfo = nullptr; // for if this is a secondary buffer
            VV_CHECK_SUCCESS(vkBeginCommandBuffer(command_buffers_[i], &command_buffer_begin_info));

            render_pass_->beginRenderPass(command_buffers_[i], VK_SUBPASS_CONTENTS_INLINE, frame_buffers_[i], swap_chain_->extent, clear_values);

            for (auto &m : models_)
                m.renderByMaterial(command_buffers_[i], general_descriptor_set_);

            render_pass_->endRenderPass(command_buffers_[i]);
            VV_CHECK_SUCCESS(vkEndCommandBuffer(command_buffers_[i]));
		}
	}
}