
#include <iostream>
#include "image/image.h"
#include "engine568.h"

int main(int argc, char ** argv) {
	if (argc != 2) {
		std::cout << "need 1 argument" << std::endl;
		return 2;
	}

	auto image = CNGE::Image::fromPNG(argv[1]);

	if (image == nullptr || !image->isValid()) {
		std::cout << "invalid filename" << std::endl;
		return 2;
	}

	auto engine = Engine568();
	engine.load(image->getWidth(), image->getHeight(), image->getPixels());

	engine.pushInt(7);
	engine.run();
	std::cout << std::endl;

	auto err = engine.getError();
	if (err != nullptr) {
		std::cout << err << std::endl;
	}

	std::cout << "Exited at " << engine.getX() << ", " << engine.getY() << std::endl;

	return 0;
}
