
#ifndef VIRTUALVISTA_SETTINGS_H
#define VIRTUALVISTA_SETTINGS_H

#include <string>

#define VV_MAX_LIGHTS 5

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
        float getAspect() const;

        std::string getApplicationName() const;
        std::string getEngineName() const;
        std::string getShaderDirectory() const;
        std::string getAssetDirectory() const;
        std::string getModelDirectory() const;

        bool isComputeRequired() const;

        uint32_t getMaxDescriptorSets() const;
        uint32_t getMaxUniformBuffers() const;
        uint32_t getMaxCombinedImageSamplers() const;

        void setWindowWidth(int width);
        void setWindowHeight(int height);

    private:
        static Settings* instance_;

        bool _default;
        int _window_width;
        int _window_height;
        float _aspect;
        std::string _application_name;
        std::string _engine_name;
        std::string _shader_directory;
        std::string _asset_directory;
        std::string _model_directory;

        bool _compute_required;

        uint32_t _max_descriptor_sets;
        uint32_t _max_uniform_buffers;
        uint32_t _max_combined_image_samplers;

        Settings() {};
        Settings(const Settings& s) {};
        Settings* operator=(const Settings& s) {};
    };
}

#endif // VIRTUALVISTA_SETTINGS_H