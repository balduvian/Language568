
//
// Created by Emmet on 1/23/2021.
//

#ifndef LANGUAGE568_ENGINE568_H
#define LANGUAGE568_ENGINE568_H

#include <vector>
#include <string>
#include <functional>

class RegisterValue {
public:
	RegisterValue();

	int integer;
	std::vector<int> * array;
};

class ValReturn {
public:
	ValReturn();
	ValReturn(int, int *, RegisterValue *);

	int val;
	int * ref;
	RegisterValue * reg;
};

class DirReturn {
public:
	DirReturn();
	DirReturn(int, int);

	int dx, dy;
};

class OpReturn {
public:
	OpReturn();
	OpReturn(bool, std::function<int(int, int)> &&);

	bool unary;
	std::function<int(int, int)> op;
};

class Engine568 {
private:
	constexpr static int NUM_REGISTERS = 6;

	constexpr static unsigned int RED = 0xFF0000;
	constexpr static unsigned int YELLOW = 0xFFFF00;
	constexpr static unsigned int GREEN = 0x00FF00;
	constexpr static unsigned int CYAN = 0x00FFFF;
	constexpr static unsigned int BLUE = 0x0000FF;
	constexpr static unsigned int MAGENTA = 0xFF00FF;

	static const char * colorNames [];

	unsigned int registerIndex;
	std::vector<RegisterValue> registers;
	std::vector<std::vector<int>> arrays;

	std::vector<unsigned int> image;
	unsigned int imageWidth, imageHeight;

	int x, y;
	int dx, dy;
	int lastValue;
	int * lastRef;
	RegisterValue * lastReg;

	bool unary;
	bool placeOperator;
	bool shouldAssign;
	std::function<int(int, int)> lastOperator;

	std::string error;

	auto outOfBounds() -> bool;
	auto getRGB() -> unsigned int;
	auto moveUntil(unsigned int &) -> bool;
	auto makeErr(std::string &&) -> void;
	auto colorName(unsigned int) -> std::string;
	auto colorIndex(unsigned int) -> unsigned int;
	auto hasError() -> bool;
	auto assignArray(unsigned int, unsigned int) -> std::vector<int> &;
	auto assign(int, int *, RegisterValue *);

	auto outOfBoundsError() -> void;

	auto parseDir() -> DirReturn;
	auto parseBranch() -> void;
	auto parseVal() -> ValReturn;
	auto parseHeap() -> void;
	auto parseOperator1() -> OpReturn;
	auto parseOperator2() -> void;

public:
	Engine568();

	auto load(unsigned int, unsigned int, unsigned char *) -> void;

	auto pushInt(int) -> void;
	auto pushArray(unsigned int, int *) -> void;

	auto run() -> void;

	auto getInt(unsigned int) -> int;
	auto getArray(unsigned int) -> std::vector<int> &;

	auto getError() -> char *;

	auto getX() -> int;
	auto getY() -> int;
};

#endif //LANGUAGE568_ENGINE568_H
