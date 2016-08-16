
#ifndef VIRTUALVISTA_SETTINGS_H
#define VIRTUALVISTA_SETTINGS_H

#include <string>

#include "Utils.h"

namespace vv 
{
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

	private:
		static Settings* instance_;

		bool default_;
		int window_width_;
		int window_height_;
		std::string application_name_;
		std::string engine_name_;
		RENDERER_TYPE renderer_type_;
		WINDOW_TYPE window_type_;

        Settings();
		Settings(const Settings& s);
		Settings* operator=(const Settings& s);
    };
}

#endif // VIRTUALVISTA_SETTINGS_H