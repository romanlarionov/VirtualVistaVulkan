
#include "Settings.h"

namespace vv
{
	Settings* Settings::instance_ = nullptr;

	void Settings::setDefault()
	{
		default = true;
		window_width = 640;
		window_height = 480;
		application_name = "VirtualVistaVulkan";

		renderer_type = VULKAN;
		window_type = GLFW;
	}

	int Settings::getWidth()
	{
		return window_width;
	}

	int Settings::getHeight()
	{
		return window_height;
	}

	std::string Settings::getName()
	{
		return application_name;
	}

	RENDERER_TYPE Settings::getRenderer()
	{
		return renderer_type;
	}

	WINDOW_TYPE Settings::getWindow()
	{
		return window_type;
	}

	Settings* Settings::inst()
	{
		if (!instance_) {
			instance_ = new Settings;
			instance_->setDefault();
		}
		return instance_;
	}

	Settings::Settings()
	{
	}
}