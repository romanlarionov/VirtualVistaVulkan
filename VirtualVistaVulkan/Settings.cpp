
#include "Settings.h"

namespace vv
{
	Settings* Settings::instance_ = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////// Public
	void Settings::setDefault()
	{
		default_ = true;
		window_width_ = 1920;
		window_height_ = 1080;
		application_name_ = "VirtualVistaVulkan";
		engine_name_ = "VirtualVista";
        asset_directory_ = "../assets/";
        //asset_directory_ = "../../assets/";
        model_directory_ = asset_directory_ + "models/";
        shader_directory_ = asset_directory_ + "shaders/";

		graphics_required_ = true;
		compute_required_ = false;
		on_screen_rendering_required_ = true;

        _max_descriptor_sets = 100;
        _max_uniform_buffers = 100;
        _max_combined_image_samplers = 100;
        _max_lights = 10;
	}


	int Settings::getWindowWidth() const
	{
		return window_width_;
	}


	int Settings::getWindowHeight() const
	{
		return window_height_;
	}


	std::string Settings::getApplicationName() const
	{
		return application_name_;
	}


	std::string Settings::getEngineName() const
	{
		return engine_name_;
	}


    std::string Settings::getShaderDirectory() const
	{
		return shader_directory_;
	}


    std::string Settings::getAssetDirectory() const
	{
		return asset_directory_;
	}


    std::string Settings::getModelDirectory() const
	{
		return model_directory_;
	}


    uint32_t Settings::getMaxDescriptorSets() const
    {
        return _max_descriptor_sets;
    }


    uint32_t Settings::getMaxUniformBuffers() const
    {
        return _max_uniform_buffers;
    }


    uint32_t Settings::getMaxCombinedImageSamplers() const
    {
        return _max_combined_image_samplers;
    }


    uint32_t Settings::getMaxLights() const
    {
        return _max_lights;
    }

	bool Settings::isGraphicsRequired() const
	{
		return graphics_required_;
	}


	bool Settings::isComputeRequired() const
	{
		return compute_required_;
	}


	bool Settings::isOnScreenRenderingRequired() const
	{
		return on_screen_rendering_required_;
	}


	void Settings::setWindowWidth(int width)
	{
		window_width_ = width;
	}


	void Settings::setWindowHeight(int height)
	{
		window_height_ = height;
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
	Settings* Settings::inst()
	{
		if (!instance_)
		{
			instance_ = new Settings;
			instance_->setDefault();
		}
		return instance_;
	}
}