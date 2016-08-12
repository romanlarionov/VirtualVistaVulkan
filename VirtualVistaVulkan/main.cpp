
#include <iostream>
#include <stdexcept>
#include <functional>

#include "App.h"
#include "VulkanRenderer.h"

using namespace vv;

int main(int argc, char **argv)
{
	App app;

	try {
		app.init();
		app.mainLoop();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}