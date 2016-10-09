
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>

#include "VulkanRenderer.h"
#include "Settings.h"

#ifdef _WIN32
#define NOMINMAX  
#include <Windows.h>
#endif

namespace vv
{
	std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0
	};

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
			// Note: this is a very specific order and is not to be messed with.
			createWindow();
			createVulkanInstance();
			createVulkanSurface();

			setupDebugCallback();
			createVulkanDevices();

			if (Settings::inst()->isOnScreenRenderingRequired())
			{
				createVulkanSwapChain();
			}

			createRenderPass();
			createGraphicsPipeline();
			createFrameBuffers();

			vertex_buffer_ = new VulkanBuffer();
			vertex_buffer_->create(physical_devices_[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(vertices[0]) * vertices.size());
			vertex_buffer_->update(vertices.data());
			vertex_buffer_->transferToDevice();

			index_buffer_ = new VulkanBuffer();
			index_buffer_->create(physical_devices_[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(indices[0]) * indices.size());
			index_buffer_->update(indices.data());
			index_buffer_->transferToDevice();

			//uniform_buffer_ = new VulkanBuffer();
			//uniform_buffer_->create();
			createCommandBuffers();
			createVulkanSemaphores();
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

		// todo: remove
		delete vertex_buffer_;
		delete index_buffer_;

		// For all physical devices
		for (std::size_t i = 0; i < physical_devices_.size(); ++i)
		{
			// Async devices
			//vkDestroyBuffer(physical_devices_[i]->logical_device, vertex_buffer_.buffer, nullptr); // todo: remove
			vkDestroySemaphore(physical_devices_[i]->logical_device, image_ready_semaphore_, nullptr);
			vkDestroySemaphore(physical_devices_[i]->logical_device, rendering_complete_semaphore_, nullptr);

			// Graphics Pipeline
			delete shader_; // todo: remove
			vkDestroyPipelineLayout(physical_devices_[i]->logical_device, pipeline_layout_, nullptr);
			vkDestroyRenderPass(physical_devices_[i]->logical_device, render_pass_, nullptr);
			vkDestroyPipeline(physical_devices_[i]->logical_device, pipeline_, nullptr);

			for (std::size_t j = 0; j < frame_buffers_.size(); ++j)
				vkDestroyFramebuffer(physical_devices_[i]->logical_device, frame_buffers_[j], nullptr);

			swap_chain_->shutDown(physical_devices_[i]);
			delete swap_chain_;

			// Physical/Logical devices
			delete physical_devices_[i];
		}

		window_->shutDown(instance_);
		delete window_;

		// Instance
		destroyDebugReportCallbackEXT(instance_, debug_callback_, nullptr);
		vkDestroyInstance(instance_, nullptr);
	}


	void VulkanRenderer::run()
	{
		// Poll window specific updates and input.
		window_->run();

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
        //if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        //    stream << "INFORMATION: ";
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


	void VulkanRenderer::createVulkanSwapChain()
	{
		swap_chain_ = new VulkanSwapChain;
		swap_chain_->create(physical_devices_[0], window_);
	}


	void VulkanRenderer::createRenderPass()
	{
		VkAttachmentDescription attachment_description = {};
		attachment_description.flags = 0;
		attachment_description.format = swap_chain_->format;
		attachment_description.samples = VK_SAMPLE_COUNT_1_BIT; // change for multi-sampling support
		attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clears the image at the beginning of each render
		attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save the image after rendering is complete
		attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // not currently using stencil
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // not currently using stencil
		attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // don't care what type of texture the framebuffer was before rendering cuz it'll be cleared anyway
		attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // finally framebuffer layout should be presentable to screen

		// todo: it's possible to have multiple sub-render-passes. You can perform things like post-processing
		// on a single framebuffer within a single RenderPass object, saving memory. Look into this for the future.

		// this handles the case for the implicit subpasses that occur for image layout transitions. this is to prevent the queue from accessing the framebuffer before its ready.
		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass = 0;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass_dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies = &subpass_dependency;

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

		// todo: this is highly specific to the shader I'm using. fix me
		vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
		vertex_input_state_create_info.vertexAttributeDescriptionCount = Vertex::getAttributeDescriptions().size();
		vertex_input_state_create_info.pVertexBindingDescriptions = &Vertex::getBindingDesciption();
		vertex_input_state_create_info.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();

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
		scissor.extent = swap_chain_->extent;

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

		// todo: for some reason, if this is activated the output color is overrided. fix me
		/* This along with color blend create info specify alpha blending operations */
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
        color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable = VK_FALSE;
        color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount = 1;
        color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

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
		//pipeline_layout_create_info.setLayoutCount = 1; // descriptor set layouts (uniform info)
		//pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout_;
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
		graphics_pipeline_create_info.pDynamicState = nullptr;//&dynamic_state_create_info;
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


	/*void VulkanRenderer::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layout_binding = {};
		layout_binding.binding = 0;
		layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layout_binding.descriptorCount = 1;

		layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // tells Vulkan where the uniform will be used. make general!!
		layout_binding.pImmutableSamplers = nullptr; // for image samplers

		VkDescriptorSetLayoutCreateInfo layout_create_info = {};
		layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_create_info.bindingCount = 1;
		layout_create_info.pBindings = &layout_binding;

		VV_CHECK_SUCCESS(vkCreateDescriptorSetLayout(physical_devices_[0]->logical_device, &layout_create_info, nullptr, &descriptor_set_layout_));


	}*/


	void VulkanRenderer::createFrameBuffers()
	{
		frame_buffers_.resize(swap_chain_->image_views.size());

		for (std::size_t i = 0; i < swap_chain_->image_views.size(); ++i)
		{
			VkImageView attachments = {
				swap_chain_->image_views[i]
			};

			VkFramebufferCreateInfo frame_buffer_create_info = {};
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.flags = 0;
			frame_buffer_create_info.renderPass = render_pass_;
			frame_buffer_create_info.attachmentCount = 1;
			frame_buffer_create_info.pAttachments = &attachments;
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

		for (std::size_t i = 0; i < command_buffers_.size(); ++i)
		{
			VkCommandBufferBeginInfo command_buffer_begin_info = {};
			command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // <- tells how long this buffer will be executed
			command_buffer_begin_info.pInheritanceInfo = nullptr; // for if this is a secondary buffer

			VV_CHECK_SUCCESS(vkBeginCommandBuffer(command_buffers_[i], &command_buffer_begin_info));

			VkRenderPassBeginInfo render_pass_begin_info = {};
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = render_pass_;
			render_pass_begin_info.framebuffer = frame_buffers_[i];
			render_pass_begin_info.renderArea.offset = { 0, 0 }; // define the size of render area
			render_pass_begin_info.renderArea.extent = swap_chain_->extent;

			VkClearValue clear_value = {0.3, 0.5, 0.5, 1.0}; // todo: offload to settings
			render_pass_begin_info.clearValueCount = 1;
			render_pass_begin_info.pClearValues = &clear_value;

			vkCmdBeginRenderPass(command_buffers_[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

			// todo: remove from single vertex buffer here
			std::array<VkDeviceSize, 1> offsets = { 0 };
			vkCmdBindVertexBuffers(command_buffers_[i], 0, 1, &vertex_buffer_->buffer, offsets.data());
			vkCmdBindIndexBuffer(command_buffers_[i], index_buffer_->buffer, 0, VK_INDEX_TYPE_UINT32);
			// todo: its recommended that you bind a single VkBuffer that contains both vertex and index data to improve cache hits

			vkCmdDraw(command_buffers_[i], vertices.size(), 1, 0, 0); // VERY instance specific! change!
			vkCmdDrawIndexed(command_buffers_[i], indices.size(), 1, 0, 0, 0);

			vkCmdEndRenderPass(command_buffers_[i]);

			VV_CHECK_SUCCESS(vkEndCommandBuffer(command_buffers_[i]));
		}
	}


	void VulkanRenderer::createVulkanSemaphores()
	{
		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VV_CHECK_SUCCESS(vkCreateSemaphore(physical_devices_[0]->logical_device, &semaphore_create_info, nullptr, &image_ready_semaphore_));
		VV_CHECK_SUCCESS(vkCreateSemaphore(physical_devices_[0]->logical_device, &semaphore_create_info, nullptr, &rendering_complete_semaphore_));
	}
}