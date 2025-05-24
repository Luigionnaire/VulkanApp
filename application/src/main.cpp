#include <iostream>
#include <stdexcept> // error handling
#include <cstdlib> // exits
#include "renderer.hpp"
#include <Windows.h>


#ifdef _DEBUG
int main()
{
	Renderer app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
#else
// For Windows applications, we need to define WinMain instead of main
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Renderer app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}



#endif