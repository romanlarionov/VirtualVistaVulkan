
#include "Settings.h"

namespace vv
{
	Settings* Settings::instance_ = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////// Public
    Settings* Settings::inst()
	{
		if (!instance_)
		{
			instance_ = new Settings;
			instance_->setDefault();
		}
		return instance_;
	}


	void Settings::setDefault()
	{
		_default = true;
		_window_width = 1920;
		_window_height = 1080;
        _aspect = _window_width / static_cast<float>(_window_height);
        
		_application_name = "VirtualVistaVulkan";
		_engine_name = "VirtualVista";
        _asset_directory = "../assets/";
        //_asset_directory = "../../assets/";
        _model_directory = _asset_directory + "models/";
        _shader_directory = _asset_directory + "shaders/";

		_compute_required = false;

        _max_descriptor_sets = 100;
        _max_uniform_buffers = 100;
        _max_combined_image_samplers = 100;
	}


	int Settings::getWindowWidth() const
	{
		return _window_width;
	}


	int Settings::getWindowHeight() const
	{
		return _window_height;
	}


    float Settings::getAspect() const
    {
        return _aspect;
    }


	std::string Settings::getApplicationName() const
	{
		return _application_name;
	}


	std::string Settings::getEngineName() const
	{
		return _engine_name;
	}


    std::string Settings::getShaderDirectory() const
	{
		return _shader_directory;
	}


    std::string Settings::getAssetDirectory() const
	{
		return _asset_directory;
	}


    std::string Settings::getModelDirectory() const
	{
		return _model_directory;
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


	bool Settings::isComputeRequired() const
	{
		return _compute_required;
	}


	void Settings::setWindowWidth(int width)
	{
		_window_width = width;
	}


	void Settings::setWindowHeight(int height)
	{
		_window_height = height;
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}