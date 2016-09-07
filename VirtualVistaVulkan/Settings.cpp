
#include "Settings.h"

namespace vv
{
	Settings* Settings::instance_ = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////// Public
	void Settings::setDefault()
	{
		default_ = true;
		window_width_ = 640;
		window_height_ = 480;
		application_name_ = "VirtualVistaVulkan";
		engine_name_ = "VirtualVista";

		renderer_type_ = VULKAN;
		window_type_ = GLFW;

		graphics_required_ = true;
		compute_required_ = false;
		on_screen_rendering_required_ = true;
	}

	int Settings::getWindowWidth()
	{
		return window_width_;
	}

	int Settings::getWindowHeight()
	{
		return window_height_;
	}

	std::string Settings::getApplicationName()
	{
		return application_name_;
	}

	std::string Settings::getEngineName()
	{
		return engine_name_;
	}

	RENDERER_TYPE Settings::getRendererType()
	{
		return renderer_type_;
	}

	WINDOW_TYPE Settings::getWindowType()
	{
		return window_type_;
	}

	bool Settings::isGraphicsRequired()
	{
		return graphics_required_;
	}

	bool Settings::isComputeRequired()
	{
		return compute_required_;
	}

	bool Settings::isOnScreenRenderingRequired()
	{
		return on_screen_rendering_required_;
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