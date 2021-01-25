
//
// Created by Emmet on 1/23/2021.
//

#include "engine568.h"

#include <iostream>

RegisterValue::RegisterValue() : integer(0), array() {}

ValReturn::ValReturn() : val(0), ref(nullptr), reg(nullptr) {}
ValReturn::ValReturn(int val, int * ref, RegisterValue * reg) : val(val), ref(ref), reg(reg) {}

DirReturn::DirReturn() : dx(0), dy(0) {}
DirReturn::DirReturn(int dx, int dy) : dx(dx), dy(dy) {}

OpReturn::OpReturn() : unary(false), op(nullptr) {}
OpReturn::OpReturn(bool unary, std::function<int(int, int)> && op) : unary(unary), op(op) {}

const char * Engine568::colorNames [6] = {
	"red",
	"yellow",
	"green",
	"cyan",
	"blue",
	"magenta"
};

Engine568::Engine568() :
	registerIndex(0),
	registers(),
	image(),
	imageWidth(0),
	imageHeight(0),
	x(0),
	y(0),
	dx(0),
	dy(0),
	lastValue(0),
	lastRef(nullptr),
	unary(false),
	placeOperator(false),
	shouldAssign(false),
	lastOperator(nullptr),
	lastReg(nullptr),
	error("")
{

}

auto Engine568::load(unsigned int width, unsigned int height, unsigned char * image) -> void {
	this->imageWidth = width;
	this->imageHeight = height;

	this->image.resize(width * height);

	for (auto i = 0u; i < width * height; ++i)
		this->image[i] = (image[i * 4] << 16u) | (image[i * 4 + 1] << 8u) | image[i * 4 + 2];

	this->registers.clear();
	this->registers.resize(NUM_REGISTERS);

	this->arrays.clear();
	this->arrays.resize(NUM_REGISTERS);

	this->registerIndex = 1;
	this->error = "";
}

auto Engine568::pushInt(int value) -> void {
	registers[registerIndex].integer = value;

	++registerIndex;
}

auto Engine568::pushArray(unsigned int length, int * data) -> void {
	auto & backingArray = assignArray(registerIndex, length);

	/* 0 out backing array */
	for (auto i = 0; i < length; ++i) backingArray[i] = data[i];

	++registerIndex;
}

auto Engine568::outOfBounds() -> bool {
	return x < 0 || y < 0 || x >= imageWidth || y >= imageHeight;
}

auto Engine568::getRGB() -> unsigned int {
	return image[y * imageWidth + x];
}

auto Engine568::moveUntil(unsigned int & rgb) -> bool {
	while (true) {
		x += dx;
		y += dy;

		if (outOfBounds()) {
			return true;

		} else {
			auto current = getRGB();

			if (current == RED || current == YELLOW || current == GREEN || current == CYAN || current == BLUE || current == MAGENTA) {
				rgb = current;
				return false;
			}
		}
	}
}

auto Engine568::makeErr(std::string && error) -> void {
	this->error = std::string("ERROR | x: ") + std::to_string(x) + " y: " + std::to_string(y);

	if (!outOfBounds()) this->error += " c: " + colorName(getRGB());

	this->error += " | " + error;
}

auto Engine568::colorName(unsigned int color) -> std::string {
	auto index = colorIndex(color);
	return index == -1 ? "unknown" : colorNames[index];
}

auto Engine568::colorIndex(unsigned int color) -> unsigned int {
	switch (color) {
		case RED: return 0;
		case YELLOW: return 1;
		case GREEN: return 2;
		case CYAN: return 3;
		case BLUE: return 4;
		case MAGENTA: return 5;
		default: return -1;
	}
}

auto Engine568::hasError() -> bool {
	return !error.empty();
}

auto Engine568::assignArray(unsigned int index, unsigned int size) -> std::vector<int> & {
	auto & reg = registers.at(index);
	auto & backingArray = arrays.at(index);

	/* allocate backing array corresponding to this register */
	backingArray.resize(size);

	/* assign this register to its array */
	reg.integer = 0;
	reg.array = arrays.data() + index;

	return backingArray;
}

auto Engine568::assign(int val, int * ref, RegisterValue * reg) {
	/* assigning to current register */
	if (reg != nullptr) {
		/* can we assign it a register? */
		if (lastReg != nullptr) {
			reg->integer = lastReg->integer;
			reg->array = lastReg->array;

		/* if not only assign value part */
		} else {
			reg->integer = lastValue;
		}
	/* assigning to current reference if an array element */
	} else if (ref != nullptr) {
		*ref = lastValue;

	} else {
		makeErr("Cannot assign to rvalue");
	}
}

auto Engine568::outOfBoundsError() -> void {
	makeErr("Out of bounds");
}

auto Engine568::parseDir() -> DirReturn {
	auto rgb = 0u;

	if (moveUntil(rgb)) return outOfBoundsError(), DirReturn();

	switch (rgb) {
		case RED: return DirReturn(1, 0);
		case YELLOW: return DirReturn(0, -1);
		case GREEN: return DirReturn(-1, 0);
		case CYAN: return DirReturn(0, 1);
		default: return makeErr("Invalid direction signifier"), DirReturn();
	}
}

auto Engine568::parseBranch() -> void {
	auto [dx, dy] = parseDir();

	if (lastValue) {
		this->dx = dx;
		this->dy = dy;
	}
}

auto Engine568::parseVal() -> ValReturn {
	auto value = 1;
	auto rgb = 0u;

	while (true) {
		if(moveUntil(rgb)) return outOfBoundsError(), ValReturn();

		switch(rgb) {
			case RED: { /* register */
				if (value != 1) return makeErr("Trying to call register value after literal signifier"), ValReturn();
				if(moveUntil(rgb)) return outOfBoundsError(), ValReturn();

				auto index = colorIndex(rgb);

				return ValReturn(registers[index].integer, &registers[index].integer, registers.data() + index);
			}
			case YELLOW: {
				if (value != 1) return makeErr("Trying to call dereferenced value after literal signifier"), ValReturn();
				if(moveUntil(rgb)) return outOfBoundsError(), ValReturn();

				auto index = colorIndex(rgb);

				auto & reg = registers.at(index);

				if (reg.array == nullptr) return makeErr(std::string("Register ") + colorNames[index] + " does not point to an array"), ValReturn();
				if (reg.integer >= reg.array->size()) return makeErr(std::string("Trying to access array ") + colorNames[index] + " out of bounds (" + std::to_string(reg.integer) + " out of " + std::to_string(reg.array->size()) + ")"), ValReturn();
				return ValReturn((*reg.array)[reg.integer], reg.array->data() + reg.integer, nullptr);
			}
			case GREEN: { /* 1 */
				value <<= 1;
				value += 1;
				break;
			}
			case CYAN: { /* 0 */
				value <<= 1;
				break;
			}
			case BLUE: { /* END */
				return ValReturn(value, nullptr, nullptr);
			}
			case MAGENTA: { /* END 0 */
				if (value == 1) {
					return ValReturn(0, nullptr, nullptr);
				} else {
					return makeErr("Invalid Zero end to nonzero value"), ValReturn();
				}
			}
		}
	}
}

auto Engine568::parseHeap() -> void {
	/* next color is the register we are allocating to */
	auto rgb = 0u;
	if (moveUntil(rgb)) return outOfBoundsError();

	auto index = colorIndex(rgb);

	/* next color block is a value, the size of the heap block we are allocating */
	auto [blockSize, ref, reg_unused] = parseVal();
	if (hasError()) return makeErr("While parsing heap block size: " + error);
	if (blockSize < 0) return makeErr("Heap block size cannot be negative");

	/* allocate */
	auto & reg = registers.at(index);
	auto & backingArray = assignArray(index, blockSize);
	/* 0 out array */
	for (auto i = 0; i < backingArray.size(); ++i) backingArray[i] = 0;

	/* initialize memory */
	for (auto element = 0;;) {
		if (moveUntil(rgb)) return outOfBoundsError();

		switch (rgb) {
			case RED: {
				auto [dx, dy] = parseDir();
				this->dx = dx;
				this->dy = dy;

				if (hasError()) return makeErr(std::string("While allocating elements for ") + colorNames[index] + ", hit direction: " + error);
				break;
			}
			case GREEN: {
				if (element == blockSize) return makeErr("Trying to allocate more than block size (" + std::to_string(blockSize) + ") for register " + colorNames[index]);

				auto [elementVal, elementRef, r_unused2] = parseVal();
				if (hasError()) return makeErr("While parsing heap block size: " + error);

				backingArray[element] = elementVal;
				++element;
				break;
			}
			case CYAN: {
				return;
			}
			default: {
				return makeErr("Ran into unexpected " + colorName(rgb) + " while allocating elements for " + colorNames[index]);
			}
		}
	}
}

auto Engine568::parseOperator1() -> OpReturn {
	auto rgb = 0u;
	if (moveUntil(rgb)) return outOfBoundsError(), OpReturn();

	switch (rgb) {
		case RED: /* + */
			return OpReturn(false, [](int last, int current) {
				return last + current;
			});
		case YELLOW: /* - */
			return OpReturn(false, [](int last, int current) {
				return last - current;
			});
		case GREEN: /* * */
			return OpReturn(false, [](int last, int current) {
				return last * current;
			});
		case CYAN: /* / */
			return OpReturn(false, [](int last, int current) {
				return last / current;
			});
		case BLUE: /* % */
			return OpReturn(false, [](int last, int current) {
				return last % current;
			});
		case MAGENTA: /* ! */
			return OpReturn(true, [](int last, int current) {
				return !last;
			});
	}

	return OpReturn();
}

auto Engine568::parseOperator2() -> void {
	auto rgb = 0u;
	if (moveUntil(rgb)) return outOfBoundsError();

	placeOperator = false;

	switch (rgb) {
		case RED: /* == */
			lastOperator = [](int last, int current) {
				return last == current;
			};
			break;
		case YELLOW: /* < */
			lastOperator = [](int last, int current) {
				return last < current;
			};
			break;
		case GREEN: /* > */
			lastOperator = [](int last, int current) {
				return last > current;
			};
			break;
		case CYAN: /* print */
			std::cout << char(lastValue);
			break;
		case BLUE: /* assignment */ {
			lastOperator = nullptr;
			shouldAssign = true;
			break;
		}
		case MAGENTA: /* =modif */ {
			placeOperator = true;

			auto [unary, op] = parseOperator1();
			if (hasError()) return makeErr("While parsing combo operator: " + error);

			if (unary) *lastRef = op(lastValue, 0);
			else lastOperator = op;

			break;
		}
	}
}

auto Engine568::run() -> void {
	lastValue = 0;
	lastRef = nullptr;
	lastReg = nullptr;

	/* first register enters as number of registers */
	registers[0].integer = registerIndex - 1;

	/* start in top left corner moving to the right */
	x = -1;
	y = 0;
	dx = 1;
	dy = 0;

	auto rgb = 0u;
	moveUntil(rgb);

	while (!outOfBounds() && !hasError()) {
		switch (rgb) {
			case RED: {
				auto [dx, dy] = parseDir();
				this->dx = dx;
				this->dy = dy;
				break;
			}
			case YELLOW:
				parseBranch();
				break;
			case GREEN: {
				auto [val, ref, reg] = parseVal();

				if (shouldAssign) {
					assign(val, ref, reg);

					shouldAssign = false;

				} else if (lastOperator != nullptr) {
					val = lastOperator(lastValue, val);
					if (placeOperator) *ref = val;

					lastOperator = nullptr;
				}

				lastValue = val;
				lastRef = ref;
				lastReg = reg;

				break;
			}
			case CYAN:
				parseHeap();
				break;
			case BLUE: {
				placeOperator = false;

				auto [unary, op] = parseOperator1();

				if (unary) *lastRef = op(lastValue, 0);
				else lastOperator = op;

				break;
			}
			case MAGENTA:
				parseOperator2();
				break;
		}

		moveUntil(rgb);
	}
}

auto Engine568::getInt(unsigned int index) -> int {
	return registers[index].integer;
}

auto Engine568::getArray(unsigned int index) -> std::vector<int> & {
	return arrays.at(index);
}

auto Engine568::getError() -> char * {
	return error.empty() ? nullptr : error.data();
}

auto Engine568::getX() -> int {
	return x;
}

auto Engine568::getY() -> int {
	return y;
}
