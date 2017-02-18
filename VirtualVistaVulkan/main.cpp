
#include <iostream>
#include <stdexcept>

#include "App.h"

using namespace vv;

int main(int argc, char **argv)
{
	App app(argc, argv);
	try
	{
		app.create();
		app.mainLoop();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	app.shutDown();

	return EXIT_SUCCESS;
}