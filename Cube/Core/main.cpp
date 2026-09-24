#include "Cube.h"

int main()
{
	try {
		Cube::cube();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Application terminated successfully\n";
	return EXIT_SUCCESS;
}