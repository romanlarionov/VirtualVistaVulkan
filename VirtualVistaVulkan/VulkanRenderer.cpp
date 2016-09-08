
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <array>

#include "VulkanRenderer.h"
#include "GLFWWindow.h"
#include "Settings.h"

#ifdef _WIN32
#define NOMINMAX  
#include <Windows.h>
#endif

namespace vv
{
	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
										  const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}


	void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
	    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	    if (func != nullptr)
	        func(instance, callback, pAllocator);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanRenderer::VulkanRenderer()
	{
	}


	VulkanRenderer::~VulkanRenderer()
	{
	}


	void VulkanRenderer::init()
	{
		try
		{
			createWindow();
			createVulkanInstance();

			if (Settings::inst()->isOnScreenRenderingRequired())
				createVulkanSurface();

			setupDebugCallback();
			createVulkanDevices();

			if (Settings::inst()->isOnScreenRenderingRequired())
			{
				createVulkanSwapChain();
				createVulkanImages();
			}

			createRenderPass();
			createGraphicsPipeline();
		}
		catch (const std::runtime_error& e)
		{
			throw e;
		}
	}


	void VulkanRenderer::shutDown()
	{
		// todo: expand for multiple possible devices.
		// todo: some of these might be null

		// Graphics Pipeline
		delete shader_; // todo: remove
		vkDestroyPipelineLayout(physical_devices_[0]->logical_device, pipeline_layout_, nullptr);
		vkDestroyRenderPass(physical_devices_[0]->logical_device, render_pass_, nullptr);
		vkDestroyPipeline(physical_devices_[0]->logical_device, pipeline_, nullptr);

		// Swap Chain
		for (int i = 0; i < swap_chain_image_views_.size(); ++i)
			vkDestroyImageView(physical_devices_[0]->logical_device, swap_chain_image_views_[i], nullptr);

		vkDestroySwapchainKHR(physical_devices_[0]->logical_device, swap_chain_, nullptr);
		vkDestroySurfaceKHR(instance_, surface_, nullptr);

		// Physical/Logical devices
		for (const auto* device : physical_devices_)
			delete device;

		// Instance
		destroyDebugReportCallbackEXT(instance_, debug_callback_, nullptr);
		vkDestroyInstance(instance_, nullptr);

		delete window_;
	}


	void VulkanRenderer::run()
	{
		// Poll window specific updates and input.
		window_->run();

		// Perform actual rendering calls
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
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 3);
		app_info.pEngineName = engine_name.c_str();
		app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		// Ensure required extensions are found.
		VV_ASSERT(checkInstanceExtensionSupport(), "Extensions requested, but are not available on this system.");

		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &app_info;

		auto required_extensions = getRequiredExtensions();
		instance_create_info.enabledExtensionCount = required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();

#ifdef _DEBUG
			instance_create_info.enabledLayerCount = used_validation_layers_.size();
			instance_create_info.ppEnabledLayerNames = used_validation_layers_.data();
#else
			instance_create_info.enabledLayerCount = 0;
#endif

		VV_CHECK_SUCCESS(vkCreateInstance(&instance_create_info, nullptr, &instance_));
	}


	void VulkanRenderer::createVulkanSurface()
	{
	    // todo: figure out if this really needs it's own function.
		VV_CHECK_SUCCESS(glfwCreateWindowSurface(instance_, window_->getHandle(), nullptr, &surface_));
	}


	std::vector<const char*> VulkanRenderer::getRequiredExtensions()
	{
	    std::vector<const char*> extensions;

		uint32_t glfw_extension_count;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// glfw specific
		for (uint32_t i = 0; i < glfw_extension_count; i++)
		    extensions.push_back(glfw_extensions[i]);

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
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
            stream << "INFORMATION: ";
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
		// Only execute if debugging to save in runtime execution processing.
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
	    // todo: implement ranking system to choose most optimal gpu or order them in increasing order of relevance.
		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr);

		VV_ASSERT(physical_device_count != 0, "Vulkan Error: no gpu with Vulkan support found");

		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());

		// Find any physical devices that might be suitable for on screen rendering.
		for (auto device : physical_devices)
		{
			VulkanDevice *vulkan_device = new VulkanDevice(device);
			VulkanSurfaceDetailsHandle surface_details_handle = {};
			if (vulkan_device->isSuitable(surface_, surface_details_handle))
			{
				vulkan_device->createLogicalDevice(true, VK_QUEUE_GRAPHICS_BIT);
				physical_devices_.push_back(vulkan_device);
				surface_settings_ = surface_details_handle;
				break; // todo: remove. for now only adding one device.
			}
		}

		VV_ASSERT(!physical_devices_.empty(), "Vulkan Error: no gpu with Vulkan support found");
	}


	VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat()
	{
		// The case when the surface has no preferred format. I choose to use standard sRGB for storage and 32 bit linear for computation.
		if ((surface_settings_.available_surface_formats.size() == 1) && surface_settings_.available_surface_formats[0].format == VK_FORMAT_UNDEFINED)
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

		// Try to find the format specified above from the list of supported formats.
		for (auto &format : surface_settings_.available_surface_formats)
			if ((format.format == VK_FORMAT_B8G8R8A8_UNORM) && (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
				return format;

		// If the desired format cannot be found, pick the first available one.
		if (!surface_settings_.available_surface_formats.empty())
			return surface_settings_.available_surface_formats[0];

		return {};
	}


	// todo: fix up needed badly. right now this only chooses the MAILBOX present mode if its found.
	// this choice should be offloaded to the settings once the available present modes are found.
	VkPresentModeKHR VulkanRenderer::chooseSwapSurfacePresentMode()
	{
		for (const auto &mode : surface_settings_.available_surface_present_modes)
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}


	VkExtent2D VulkanRenderer::chooseSwapSurfaceExtent()
	{
		// if the extent is the max uint32_t, it means that the resolution of the swap chain cant be anything.
		if (surface_settings_.surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return surface_settings_.surface_capabilities.currentExtent;

		VkExtent2D extent = {};
		extent.width = std::max(surface_settings_.surface_capabilities.minImageExtent.width, std::min(surface_settings_.surface_capabilities.maxImageExtent.width, (uint32_t)Settings::inst()->getWindowWidth()));
		extent.height = std::max(surface_settings_.surface_capabilities.minImageExtent.height, std::min(surface_settings_.surface_capabilities.maxImageExtent.height, (uint32_t)Settings::inst()->getWindowHeight()));
		return extent;
	}


	// todo: this could be altered to allow for on-the-fly graphics configuration without a need for a hard restart.
	// for now, I'm setting the swap chain to be initialized with whatever the default settings are on renderer initialization.
	void VulkanRenderer::createVulkanSwapChain()
	{
		VV_ASSERT(physical_devices_[0]->logical_device, "Logical device not present");
		VulkanSurfaceDetailsHandle details = surface_settings_;
		VkSurfaceFormatKHR chosen_format = chooseSwapSurfaceFormat();
		VkPresentModeKHR chosen_present_mode = chooseSwapSurfacePresentMode();
		swap_chain_extent_ = chooseSwapSurfaceExtent();
		swap_chain_format_ = chosen_format.format;

		// Queue length for swap chain. (How many images are kept waiting).
		uint32_t image_count = details.surface_capabilities.minImageCount;
		if ((details.surface_capabilities.maxImageCount > 0) && (image_count > details.surface_capabilities.maxImageCount)) // if 0, maxImageCount doesn't have a limit.
			image_count = details.surface_capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR swap_chain_create_info = {};
		swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swap_chain_create_info.flags = 0;
		swap_chain_create_info.surface = surface_;
		swap_chain_create_info.minImageCount = image_count;
		swap_chain_create_info.imageFormat = chosen_format.format;
		swap_chain_create_info.imageColorSpace = chosen_format.colorSpace;
		swap_chain_create_info.imageExtent = swap_chain_extent_;
		swap_chain_create_info.imageArrayLayers = 1; // todo: make dynamic. 2 would be for VR support
		swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT <- to do post processing

		if (physical_devices_[0]->graphics_family_index != physical_devices_[0]->display_family_index)
		{
			uint32_t queue[] = { (uint32_t)physical_devices_[0]->graphics_family_index, (uint32_t)(physical_devices_[0]->display_family_index) };
			swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swap_chain_create_info.queueFamilyIndexCount = 2; // display and graphics use two different queues
			swap_chain_create_info.pQueueFamilyIndices = queue;
		}
		else
		{
			swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swap_chain_create_info.queueFamilyIndexCount = 0;
			swap_chain_create_info.pQueueFamilyIndices = nullptr;
		}

		swap_chain_create_info.preTransform = surface_settings_.surface_capabilities.currentTransform;
		swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swap_chain_create_info.presentMode = chosen_present_mode;
		swap_chain_create_info.clipped = VK_TRUE;
		swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

		VV_CHECK_SUCCESS(vkCreateSwapchainKHR(physical_devices_[0]->logical_device, &swap_chain_create_info, nullptr, &swap_chain_));
	}


	void VulkanRenderer::createVulkanImages()
	{
		// This is effectively creating a queue of frames to be displayed. 
		// todo: support VR by having multiple lists containing images/image views for each eye.

		uint32_t swap_chain_image_count = 0;
		VV_CHECK_SUCCESS(vkGetSwapchainImagesKHR(physical_devices_[0]->logical_device, swap_chain_, &swap_chain_image_count, nullptr));
		swap_chain_images_.resize(swap_chain_image_count);
		VV_CHECK_SUCCESS(vkGetSwapchainImagesKHR(physical_devices_[0]->logical_device, swap_chain_, &swap_chain_image_count, swap_chain_images_.data()));

		swap_chain_image_views_.resize(swap_chain_image_count);
		for (int i = 0; i < swap_chain_image_count; ++i)
		{
			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.flags = VK_NULL_HANDLE;
			image_view_create_info.image = swap_chain_images_[i];
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = swap_chain_format_;

			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;

			VkImageView image_view = {};
			VV_CHECK_SUCCESS(vkCreateImageView(physical_devices_[0]->logical_device, &image_view_create_info, nullptr, &image_view));
			swap_chain_image_views_[i] = image_view;
		}
	}


	void VulkanRenderer::createRenderPass()
	{
		VkAttachmentDescription attachment_description = {};
		attachment_description.flags = 0;
		attachment_description.format = swap_chain_format_;
		attachment_description.samples = VK_SAMPLE_COUNT_1_BIT; // change for multi-sampling support
		attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clears the image at the beginning of each render
		attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save the image after rendering is complete
		attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // not currently using stencil
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // not currently using stencil
		attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // dont care what type of texture the framebuffer was before rendering cuz it'll be cleared anyway
		attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // finaly framebuffer layout should be presentable to screen

		// todo: it's possible to have multiple sub-render-passes. You can perform things like post-processing
		// on a single framebuffer within a single RenderPass object, saving memory. Look into this for the future.

		VkAttachmentReference attachment_reference = {};
		attachment_reference.attachment = 0; // framebuffer at index 0
		attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // use internal framebuffer as color texture

		VkSubpassDescription subpass_description = {};
		subpass_description.flags = 0;
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // this will be a graphics subpass
		subpass_description.colorAttachmentCount = 1; // one framebuffer
		subpass_description.pColorAttachments = &attachment_reference;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.flags = 0;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments = &attachment_description;
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass_description;

		VV_CHECK_SUCCESS(vkCreateRenderPass(physical_devices_[0]->logical_device, &render_pass_create_info, nullptr, &render_pass_));
	}


	void VulkanRenderer::createGraphicsPipeline()
	{
		shader_ = new Shader;
		shader_->create("D:/Developer/VirtualVistaVulkan/VirtualVistaVulkan/", "triangle", physical_devices_[0]->logical_device);

		VkPipelineShaderStageCreateInfo vert_shader_create_info = {};
		vert_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

		vert_shader_create_info.module = shader_->vert_module;
		vert_shader_create_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_create_info = {};
		frag_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		frag_shader_create_info.module = shader_->frag_module;
		frag_shader_create_info.pName = "main";

		std::size_t two = 2;
		std::array<VkPipelineShaderStageCreateInfo, 2> shaders = { vert_shader_create_info, frag_shader_create_info };

		/* Fixed Function Pipeline Layout */
		VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
		vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_create_info.flags = 0;
		vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
		vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
		vertex_input_state_create_info.pVertexAttributeDescriptions = nullptr;
		vertex_input_state_create_info.pVertexBindingDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
		input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.flags = 0;
		input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // render triangles
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.width = Settings::inst()->getWindowWidth();
		viewport.height = Settings::inst()->getWindowHeight();
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swap_chain_extent_;

		VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
		viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_create_info.flags = 0;
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount = 1;
		viewport_state_create_info.pViewports = &viewport;
		viewport_state_create_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
		rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info.depthClampEnable = VK_FALSE; // clamp geometry within clip space
		rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE; // discard geometry
		rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL; // create fragments from the inside of a polygon
		rasterization_state_create_info.lineWidth = 1.0f;
		rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT; // cull the back of polygons from rendering
		rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // order of vertices
		rasterization_state_create_info.depthBiasEnable = VK_FALSE; // all stuff for shadow mapping? look into it
		rasterization_state_create_info.depthBiasClamp = 0.0f;
		rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
		rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

		// todo: add anti-aliasing settings support
		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
		multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create_info.flags = 0;
		multisample_state_create_info.sampleShadingEnable = VK_FALSE;
		multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_create_info.minSampleShading = 1.0f;
		multisample_state_create_info.pSampleMask = nullptr;
		multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_state_create_info.alphaToOneEnable = VK_FALSE;

		/* This along with color blend create info specify alpha blending operations */
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.blendEnable = VK_TRUE;
		color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
		color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_create_info.flags = 0;
		color_blend_state_create_info.logicOpEnable = VK_FALSE;
		color_blend_state_create_info.attachmentCount = 1;
		color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

		// add enum values here for more dynamic pipeline state changes!! 
		std::array<VkDynamicState, 2> dynamic_pipeline_settings = { VK_DYNAMIC_STATE_VIEWPORT };

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.flags = 0;
		dynamic_state_create_info.dynamicStateCount = dynamic_pipeline_settings.size();
		dynamic_state_create_info.pDynamicStates = dynamic_pipeline_settings.data();

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.flags = 0;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = nullptr;
		pipeline_layout_create_info.pPushConstantRanges = nullptr;
		pipeline_layout_create_info.pushConstantRangeCount = 0;

		VV_CHECK_SUCCESS(vkCreatePipelineLayout(physical_devices_[0]->logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout_));

		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
		graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.flags = 0;
		graphics_pipeline_create_info.stageCount = 2; // vert & frag shader
		graphics_pipeline_create_info.pStages = shaders.data();
		graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
		graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState = nullptr;
		graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
		graphics_pipeline_create_info.layout = pipeline_layout_;
		graphics_pipeline_create_info.renderPass = render_pass_;
		graphics_pipeline_create_info.subpass = 0; // index of framebuffer used for graphics
		graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE; // used for creating new pipeline from existing one.
		graphics_pipeline_create_info.basePipelineIndex = -1; // set to nothing for now cuz only single pipeline

		// todo: this call can create multiple pipelines with a single call. utilize to improve performance.
		// info: the null handle here specifies a VkPipelineCache that can be used to store pipeline creation info after a pipeline's deletion.
		VV_CHECK_SUCCESS(vkCreateGraphicsPipelines(physical_devices_[0]->logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline_));
	}
}