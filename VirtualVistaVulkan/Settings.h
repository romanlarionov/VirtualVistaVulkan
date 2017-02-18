
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

        int getWindowWidth() const;
        int getWindowHeight() const;
        std::string getApplicationName() const;
        std::string getEngineName() const;
        std::string getShaderDirectory() const;
        std::string getAssetDirectory() const;
        std::string getModelDirectory() const;

        bool isGraphicsRequired() const;
        bool isComputeRequired() const;
        bool isOnScreenRenderingRequired() const;

        uint32_t getMaxDescriptorSets() const;
        uint32_t getMaxUniformBuffers() const;
        uint32_t getMaxCombinedImageSamplers() const;
        uint32_t getMaxLights() const;

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
        std::string model_directory_;

        bool graphics_required_;
        bool compute_required_;
        bool on_screen_rendering_required_;

        uint32_t _max_descriptor_sets;
        uint32_t _max_uniform_buffers;
        uint32_t _max_combined_image_samplers;
        uint32_t _max_lights;

        Settings() {};
        Settings(const Settings& s) {};
        Settings* operator=(const Settings& s) {};
    };
}

#endif // VIRTUALVISTA_SETTINGS_H