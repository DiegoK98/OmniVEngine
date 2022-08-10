#include "OmniVEngineApp.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
	OmniV::OmniVEngineApp app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		// std::cerr << "Invalid arguments: Expected *.xml file as only argument" << std::endl; // For when loadScene throws exception
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}