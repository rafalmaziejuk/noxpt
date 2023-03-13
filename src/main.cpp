#include "application.h"

int main(int argc, char **argv) {
	auto application = NOXPT::Application::createApplication({ argc, argv });
	application->run();
	delete application;
	application = nullptr;

	return 0;
}
