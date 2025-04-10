#include <iostream>
#include <stdexcept> // error handling
#include <cstdlib> // exits
#include "triangle.hpp"

int main()
{
	Triangle app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}