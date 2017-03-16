
#include <iostream>
#include <stdexcept>

#include "VirtualVistaEngine.h"

using namespace vv;

int main(int argc, char **argv)
{
    VirtualVistaEngine app(argc, argv);
    try
    {
    	app.create();
    }
    catch (const std::runtime_error& e)
    {
    	std::cerr << e.what() << std::endl;
    	return EXIT_FAILURE;
    }

    Scene *scene = app.getScene();

    Camera *camera = scene->addCamera(glm::radians(90.0f), 0.1f, 1000.0f);
    scene->setActiveCamera(camera);
    camera->translate(glm::vec3(0.0f, 0.0f, -9.0f));

    SkyBox *skybox = scene->addSkyBox("Canyon/", "Unfiltered_HDR.dds", "Diffuse_HDR.dds", "Specular_HDR.dds", "FSchlick_DGGX_GSmith.dds");
    scene->setActiveSkyBox(skybox);

    /*Light *light = scene->addLight(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), 2.0f);
    light->translate(glm::vec3(2.0f, 1.5f, 0.0f));

    Light *light2 = scene->addLight(glm::vec4(1.0f, 0.1f, 0.1f, 0.0f), 3.0f);
    light2->translate(glm::vec3(0.0f, 1.5f, 1.0f));

    Light *light3 = scene->addLight(glm::vec4(0.1f, 0.1f, 1.0f, 0.0f), 2.0f);
    light3->translate(glm::vec3(0.0f, 1.5f, -1.0f));*/

    ///Model *model = scene->addModel("hammardillo/", "hammardillo.obj", "triangle");
    //Model *model = scene->addModel("chalet/", "chalet.obj", "dummy");
    //Model *model = scene->addModel("sponza/", "sponza.obj", "phong");
    //Model *model = scene->addModel("contemporary_bathroom/", "contemporary_bathroom.obj", "phong");
    ///model->rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ///model->rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //model->scale(glm::vec3(0.01f, 0.01f, 0.01f));
    ///model->scale(glm::vec3(0.5f, 0.5f, 0.5f));

    auto input_handler = [](Scene *scene, float delta_time)
    {
        float move_speed = 12.0f * delta_time;
        float rotate_speed = 2.0f * delta_time;
        Camera *camera = scene->getActiveCamera();
        if (InputManager::inst()->keyIsPressed(GLFW_KEY_W))
            camera->translate(move_speed * camera->getForwardDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_A))
            camera->translate(-move_speed * camera->getSidewaysDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_S))
            camera->translate(-move_speed * camera->getForwardDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_D))
            camera->translate(move_speed * camera->getSidewaysDirection());

        double delta_x, delta_y;
        InputManager::inst()->getCursorGradient(delta_x, delta_y);
        camera->rotate(delta_x * rotate_speed, delta_y * rotate_speed);
    };

    app.beginMainLoop(input_handler);
    app.shutDown();

    return EXIT_SUCCESS;
}