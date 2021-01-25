
#include "imageUtil.h"

#include <cmath>

namespace CNGE::Util {
	auto red(u32 pixel) -> u8 {
		return u8((pixel >> 24) & 0xff);
	}

	auto gre(u32 pixel) -> u8 {
		return u8((pixel >> 16) & 0xff);
	}

	auto blu(u32 pixel) -> u8 {
		return u8((pixel >> 8) & 0xff);
	}

	auto alp(u32 pixel) -> u8 {
		return u8((pixel) & 0xff);
	}

	auto pix(const u8 red, const u8 gre, const u8 blu, const u8 alp) -> u32 {
		return u32((red << 24) | (gre << 16) | (blu << 8) | alp);
	}

	auto pix(const u8 red, const u8 gre, const u8 blu) -> u32 {
		return u32((red << 24) | (gre << 16) | (blu << 8) | 0xff);
	}

	auto pix(const u32 pixel) -> channelReturn {
		return channelReturn{
			red(pixel),
			gre(pixel),
			blu(pixel),
			alp(pixel)
		};
	}

	auto pos(u32 x, u32 y, u32 width) -> u32 {
		return u32(y * width + x);
	}

	auto difference(u32 pixel0, u32 pixel1) -> u32 {
		auto [red0, gre0, blu0, alp0] = pix(pixel0);
		auto [red1, gre1, blu1, alp1] = pix(pixel1);

		return u32(abs(red1 - red0) + abs(gre1 - gre0) + abs(blu1 - blu0));
	}

	auto difference(u8 red, u8 gre, u8 blu, u32 pixel) -> u32 {
		auto [red1, gre1, blu1, alp1] = pix(pixel);

		return u32(abs(red1 - red) + abs(gre1 - gre) + abs(blu1 - blu));
	}

	auto swapBuffers(Image** image0, Image** image1) -> void {
		auto temp = *image0;
		*image0 = *image1;
		*image1 = temp;
	}

	auto mod(int a, unsigned b) -> int {
		int mod = a % (int)b;

		if (a < 0) mod += b;

		return mod;
	}

	auto mix(u32 color0, u32 color1, float distribution) -> u32 {
		return pix(
			u8(red(color0) * (1.0 - distribution) + red(color1) * distribution),
			u8(gre(color0) * (1.0 - distribution) + gre(color1) * distribution),
			u8(blu(color0) * (1.0 - distribution) + blu(color1) * distribution),
			u8(alp(color0) * (1.0 - distribution) + alp(color1) * distribution)
		);
	}

	auto conformToRange(int val, int low, int high) -> int {
		if (val < low)
			val = low;
		else if (val > high)
			val = high;

		return val;
	}

	auto addNoise(u32 pixel, int amount) -> u32 {
		auto red = conformToRange(Util::red(pixel) + amount, 0x00, 0xff);
		auto gre = conformToRange(Util::gre(pixel) + amount, 0x00, 0xff);
		auto blu = conformToRange(Util::blu(pixel) + amount, 0x00, 0xff);

		return Util::pix(red, gre, blu, Util::alp(pixel));
	}

	auto addNoise(u32 pixel, int amountR, int amountG, int amountB) -> u32 {
		auto red = conformToRange(Util::red(pixel) + amountR, 0x00, 0xff);
		auto gre = conformToRange(Util::gre(pixel) + amountG, 0x00, 0xff);
		auto blu = conformToRange(Util::blu(pixel) + amountB, 0x00, 0xff);

		return Util::pix(red, gre, blu, Util::alp(pixel));
	}

	auto luminance(u32 pixel) -> int {
		return Util::red(pixel) + Util::gre(pixel) + Util::blu(pixel);
	}

	auto interp(float min, float max, float along) -> float {
		return (max - min) * along + min;
	}

	auto smallBound(int x) -> int {
		if (x < 0) x = 0;
		return x;
	}

	auto largeBound(int x, u32 width) -> int {
		if (x > width) x = width;
		return x;
	}

	auto matchSize(Image** imageFrom, Image** imageTo) -> void {
		(*imageTo)->resize((*imageFrom)->getWidth(), (*imageFrom)->getHeight());
	}

	auto mode(Image** imageFrom, Image** imageTo, u32* colors, int numColors, int radius) -> void {
		auto width = (*imageFrom)->getWidth();
		auto height = (*imageFrom)->getHeight();
		auto counts = new u64[numColors];

		auto pixelsFrom = (*imageFrom)->getPixels();
		auto   pixelsTo = (*  imageTo)->getPixels();

		/* now place back in the colors based on a mode check */
		for (auto i = 0u; i < width; ++i) {
			for (auto j = 0u; j < height; ++j) {
				/* reuse counts for mode check */
				for (auto i = 0; i < numColors; ++i)
					counts[i] = 0;

				auto left = Util::smallBound(i - radius);
				auto right = Util::largeBound(i + radius + 1, width);
				auto up = Util::smallBound(j - radius);
				auto down = Util::largeBound(j + radius + 1, height);

				/* count how many of each color is around this pixel */
				for (auto k = left; k < right; ++k)
					for (auto l = up; l < down; ++l)
						++counts[pixelsFrom[Util::pos(k, l, width)] >> 8];

				auto index = 0;
				auto highestCount = 0;

				/* find the mode of the colors around this pixel */
				for (auto k = 0; k < numColors; ++k) {
					if (counts[k] > highestCount) {
						highestCount = counts[k];
						index = k;
					}
				}

				auto pos = Util::pos(i, j, width);
				pixelsTo[pos] = (colors[index] & 0xffffff00) | Util::alp(pixelsFrom[pos]);
			}
		}

		delete[] counts;
	}

	auto copy(Image* from, Image* to) -> void {
		auto size = from->getWidth() * from->getWidth();

		auto pixelsFrom = from->getPixels();
		auto pixelsTo = to->getPixels();

		for (auto i = 0; i < size; ++i) {
			pixelsTo[i] = pixelsFrom[i];
		}
	}

	namespace sample {
		auto at(u32 * src, u32 x, u32 y, u32 width, u32 height, u32 fallbackColor) -> u32 {
			if (x >= width || y >= height) {
				return fallbackColor;
			} else {
				return src[Util::pos(x, y, width)];
			}
		}

		auto nearest(u32 * src, float x, float y, u32 width, u32 height, u32 fallbackColor) -> u32 {
			return at(src, u32(round(x)), u32(round(y)), width, height, fallbackColor);
		}

		auto bilinear(u32 * src, float x, float y, u32 width, u32 height, u32 fallbackColor) -> u32 {
			auto color0 = at(src, u32(floor(x)), u32(floor(y)), width, height, fallbackColor);
			auto color1 = at(src, u32(ceil(x)),  u32(floor(y)), width, height, fallbackColor);
			auto color2 = at(src, u32(floor(x)), u32(ceil(y)),  width, height, fallbackColor);
			auto color3 = at(src, u32(ceil(x)),  u32(ceil(y)),  width, height, fallbackColor);

			auto horizontalDistribution = x - floor(x);
			auto verticalDistribution   = y - floor(y);

			return mix(
				mix(color0, color1, horizontalDistribution),
				mix(color2, color3, horizontalDistribution),
				verticalDistribution
			);
		}
	}
}
