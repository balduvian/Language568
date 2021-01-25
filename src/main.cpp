
#include <iostream>
#include "image/image.h"
#include "engine568.h"

int main() {
	auto image = CNGE::Image::fromPNG("../helloWorld.png");

	auto engine = Engine568();
	engine.load(image->getWidth(), image->getHeight(), image->getPixels());

	engine.pushInt(7);
	engine.run();

	auto err = engine.getError();
	if (err != nullptr) {
		std::cout << err << std::endl;
	}

	std::cout << engine.getX() << " | " << engine.getY() << std::endl;

	return 0;
}
