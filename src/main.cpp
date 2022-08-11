#include "OmniVEngineApp.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
	if (argc > 2)
	{
		std::cerr << "Invalid number of arguments" << std::endl;
		return EXIT_FAILURE;
	}

	std::string sceneFile;
	if (argc != 1)
		sceneFile = std::string(argv[1]);
	else
		sceneFile = "testScene.xml"; // Default scene to load if no argument given

	OmniV::OmniVEngineApp app{};

	try {
		app.loadScene(sceneFile);
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		// std::cerr << "Invalid arguments: Expected *.xml file as only argument" << std::endl; // For when loadScene throws exception
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}