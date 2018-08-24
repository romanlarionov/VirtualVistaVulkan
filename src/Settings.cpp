
#include "Settings.h"

namespace vv
{
    Settings* Settings::m_instance = nullptr;

    Settings* Settings::inst()
    {
        if (!m_instance)
        {
            m_instance = new Settings;
            m_instance->setDefault();
        }
        return m_instance;
    }


    void Settings::setDefault()
    {
        m_default           = true;
        m_window_width      = 1920;
        m_window_height     = 1080;
        m_aspect            = m_window_width / static_cast<float>(m_window_height);
        m_application_name  = "VirtualVistaVulkan";
        m_engine_name       = "VirtualVista";
        m_asset_directory   = ROOTPROJECTDIR "/assets/";
        m_model_directory   = m_asset_directory + "models/";
        m_texture_directory = m_asset_directory + "textures/";
        m_shader_directory  = m_asset_directory + "shaders/";
        
        m_compute_required  = false;

        m_max_descriptor_sets = 100;
        m_max_uniform_buffers = 100;
        m_max_combined_image_samplers = 100;
    }


    int Settings::getWindowWidth() const
    {
        return m_window_width;
    }


    int Settings::getWindowHeight() const
    {
        return m_window_height;
    }


    float Settings::getAspect() const
    {
        return m_aspect;
    }


    std::string Settings::getApplicationName() const
    {
        return m_application_name;
    }


    std::string Settings::getEngineName() const
    {
        return m_engine_name;
    }


    std::string Settings::getShaderDirectory() const
    {
        return m_shader_directory;
    }


    std::string Settings::getAssetDirectory() const
    {
        return m_asset_directory;
    }


    std::string Settings::getModelDirectory() const
    {
        return m_model_directory;
    }


    std::string Settings::getTextureDirectory() const
    {
        return m_texture_directory;
    }


    uint32_t Settings::getMaxDescriptorSets() const
    {
        return m_max_descriptor_sets;
    }


    uint32_t Settings::getMaxUniformBuffers() const
    {
        return m_max_uniform_buffers;
    }


    uint32_t Settings::getMaxCombinedImageSamplers() const
    {
        return m_max_combined_image_samplers;
    }


    bool Settings::isComputeRequired() const
    {
        return m_compute_required;
    }


    void Settings::setWindowWidth(int width)
    {
        m_window_width = width;
    }


    void Settings::setWindowHeight(int height)
    {
        m_window_height = height;
    }
}