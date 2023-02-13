#include "EngineApp.hpp"

int main(int argc, char* argv[]) {
	// This path is not generic. Can't find a proper solution
	std::filesystem::current_path("D:/Documents/OmniVEngine/");

	if (argc > 2)
	{
		std::cerr << "Invalid number of arguments" << std::endl;
		return EXIT_FAILURE;
	}

	std::string sceneFile;
	if (argc == 2)
		sceneFile = std::string(argv[1]);
	else
		sceneFile = "testScene.xml"; // Default scene to load if no argument given

	OmniV::EngineApp app;

	try {
		app.loadScene(sceneFile);
		app.run();
	}
	catch (const std::exception& e) {
		LOGERROR(e.what());
		// LOGERROR("Invalid arguments: Expected *.xml file as only argument"); // For when loadScene throws exception
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}