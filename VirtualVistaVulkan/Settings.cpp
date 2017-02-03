
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
        shader_directory_ = "D:/Developer/VirtualVistaVulkan/VirtualVistaVulkan/"; // todo: change!
        asset_directory_ = "../assets/";
        model_directory_ = asset_directory_ + "models/";

		graphics_required_ = true;
		compute_required_ = false;
		on_screen_rendering_required_ = true;
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