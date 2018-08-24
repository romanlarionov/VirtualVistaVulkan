
#ifndef VIRTUALVISTA_SETTINGS_H
#define VIRTUALVISTA_SETTINGS_H

#include <string>

#define VV_MAX_LIGHTS 5

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
        std::string getTextureDirectory() const;

        bool isComputeRequired() const;

        uint32_t getMaxDescriptorSets() const;
        uint32_t getMaxUniformBuffers() const;
        uint32_t getMaxCombinedImageSamplers() const;

        void setWindowWidth(int width);
        void setWindowHeight(int height);

    private:
        static Settings* m_instance;

        bool m_default;
        int m_window_width;
        int m_window_height;
        float m_aspect;
        std::string m_application_name;
        std::string m_engine_name;
        std::string m_shader_directory;
        std::string m_asset_directory;
        std::string m_model_directory;
        std::string m_texture_directory;

        bool m_compute_required;

        uint32_t m_max_descriptor_sets;
        uint32_t m_max_uniform_buffers;
        uint32_t m_max_combined_image_samplers;

        Settings() {};
        Settings(const Settings& s) {};
        Settings* operator=(const Settings& s) {};
    };
}

#endif // VIRTUALVISTA_SETTINGS_H