
#include <iostream>
#include <stdexcept>

#include "App.h"

using namespace vv;

int main(int argc, char **argv)
{
	try
	{
		App app(argc, argv);
		app.mainLoop();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}