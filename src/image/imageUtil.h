
#ifndef FPP_UTIL
#define FPP_UTIL

#include <cstdint>
#include "types.h"
#include "image.h"

namespace CNGE {
	namespace Util {
		template <typename T>
		auto PI = T(3.141592653589793238462643383279502884);

		struct channelReturn {
			u8 red;
			u8 gre;
			u8 blu;
			u8 alp;
		};
		
		extern auto red(u32) -> u8;
		extern auto gre(u32) -> u8;
		extern auto blu(u32) -> u8;
		extern auto alp(u32) -> u8;
		
		extern auto pix(u8, u8, u8, u8) -> u32;
		extern auto pix(u8, u8, u8) -> u32;
		extern auto pix(u32) -> channelReturn;
		
		extern auto pos(u32, u32, u32) -> u32;

		extern auto difference(u32, u32) -> u32;
		extern auto difference(u8, u8, u8, u32) -> u32;

		extern auto swapBuffers(Image**, Image**) -> void;

		extern auto mod(int, unsigned) -> int;

		auto mix(u32 color0, u32 color1, float distribution) -> u32;

		extern auto conformToRange(int, int, int) -> int;

		extern auto addNoise(u32, int) -> u32;
		extern auto addNoise(u32, int, int, int) -> u32;

		constexpr auto MAX_LUMINANCE = 0xff * 3;

		extern auto luminance(u32) -> int;

		extern auto interp(float, float, float) -> float;

		extern auto smallBound(int) -> int;
		extern auto largeBound(int, u32) -> int;

		extern auto matchSize(Image**, Image**) -> void;

		extern auto mode(Image**, Image**, u32*, int, int) -> void;

		extern auto copy(Image*, Image*) -> void;

		template <size_t N, typename T>
		constexpr auto size(T (&array)[N]) {
			return N;
		}

		namespace sample {
			extern auto at(u32 * src, u32 x, u32 y, u32 width, u32 height, u32 fallbackColor) -> u32;
			extern auto nearest(u32 * src, float x, float y, u32 width, u32 height, u32 fallbackColor) -> u32;
			extern auto bilinear(u32 * src, float x, float y, u32 width, u32 height, u32 fallbackColor) -> u32;
		}
	}
}

#endif
