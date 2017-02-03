
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
        std::string getShaderDirectory();
        std::string getAssetDirectory();

        bool isGraphicsRequired();
        bool isComputeRequired();
        bool isOnScreenRenderingRequired();

        void setWindowWidth(int width);
        void setWindowHeight(int height);

    private:
        static Settings* instance_;

        bool default_;
        int window_width_;
        int window_height_;
        std::string application_name_;
        std::string engine_name_;
        std::string shader_directory_;
        std::string asset_directory_;

        bool graphics_required_;
        bool compute_required_;
        bool on_screen_rendering_required_;

        Settings() {};
        Settings(const Settings& s) {};
        Settings* operator=(const Settings& s) {};
    };
}

#endif // VIRTUALVISTA_SETTINGS_H