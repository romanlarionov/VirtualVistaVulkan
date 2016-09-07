
#ifndef VIRTUALVISTA_SETTINGS_H
#define VIRTUALVISTA_SETTINGS_H

#include <string>

#include "Utils.h"

namespace vv 
{
	// todo: offload default settings to file. Read at application start.
    class Settings 
    {
    public:
        static Settings* inst();
		void setDefault();

	int getWindowWidth();
	int getWindowHeight();
	std::string getApplicationName();
	std::string getEngineName();
	RENDERER_TYPE getRendererType();
	WINDOW_TYPE getWindowType();
	bool isGraphicsRequired();
	bool isComputeRequired();
	bool isOnScreenRenderingRequired();

    private:
	static Settings* instance_;

	bool default_;
	int window_width_;
	int window_height_;
	std::string application_name_;
	std::string engine_name_;

	WINDOW_TYPE window_type_;

	// Graphics Settings
	RENDERER_TYPE renderer_type_;

	bool graphics_required_;
	bool compute_required_;
	bool on_screen_rendering_required_;

        Settings() {};
        Settings(const Settings& s) {};
        Settings* operator=(const Settings& s) {};
    };
}

#endif // VIRTUALVISTA_SETTINGS_H
