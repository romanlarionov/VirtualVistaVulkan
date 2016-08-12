
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

		int getWidth();
		int getHeight();
		std::string getName();
		RENDERER_TYPE getRenderer();
		WINDOW_TYPE getWindow();

	private:
		static Settings* instance_;

		bool default;
		int window_width;
		int window_height;
		std::string application_name;
		RENDERER_TYPE renderer_type;
		WINDOW_TYPE window_type;

        Settings();
		Settings(const Settings& s);
		Settings* operator=(const Settings& s);
    };
}

#endif // VIRTUALVISTA_SETTINGS_H