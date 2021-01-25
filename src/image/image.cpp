
#include <iostream>
#include <stdio.h>
#include <memory>
#include <setjmp.h>

#include "image.h"

#include "libpng16/png.h"

#include "imageUtil.h"

namespace CNGE {
	Image::Image(): pixels(nullptr) {}

	Image::Image(u32 width, u32 height, u8* pixels)
		: width(width), height(height), pixels(pixels) {}

	Image::Image(Image&& other) : width(other.width), height(other.height), pixels(other.pixels) {
		other.pixels = nullptr;
	}

	auto Image::operator=(Image&& other) -> Image& {
		width = other.width;
		height = other.height;
		pixels = other.pixels;
		other.pixels = nullptr;

		return *this;
	}

	Image::~Image() {
		delete[] pixels;
	}

	auto Image::fromPNG(const char *filepath) -> std::unique_ptr<Image> {
		auto* file = static_cast<FILE*>(nullptr);

		/* open the file of the image */
		fopen_s(&file, filepath, "rb");

		if (!file) return nullptr;

		auto* png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		auto* info = png_create_info_struct(png);

		png_init_io(png, file);
		png_read_info(png, info);

		auto width = png_get_image_width(png, info);
		auto height = png_get_image_height(png, info);
		auto* pixels = new u8[u64(width) * height * 4];

		/* convert the image into 8 bit rgba */
		const auto colorType = png_get_color_type(png, info);
		const auto bitDepth = png_get_bit_depth(png, info);

		if (bitDepth == 16)
			png_set_strip_16(png);

		if (colorType == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png);

		if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
			png_set_expand_gray_1_2_4_to_8(png);

		if (png_get_valid(png, info, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png);

		if (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_PALETTE)
			png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

		if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png);

		png_read_update_info(png, info);

		/* cache the row so we can convert it */
		auto* pngRow = new u8[width * 4llu];

		/* read each row into the 1d array */
		for (auto j = 0u; j < height; ++j) {
			png_read_row(png, pngRow, nullptr);

			for (auto i = 0u; i < width; ++i) {
				pixels[(j * width + i) * 4    ] = pngRow[i * 4    ];
				pixels[(j * width + i) * 4 + 1] = pngRow[i * 4 + 1];
				pixels[(j * width + i) * 4 + 2] = pngRow[i * 4 + 2];
				pixels[(j * width + i) * 4 + 3] = pngRow[i * 4 + 3];
			}
		}

		delete[] pngRow;

		png_destroy_read_struct(&png, &info, nullptr);
		fclose(file);

		return std::make_unique<Image>(width, height, pixels);
	}

	auto Image::makeSheet(u32 width, u32 height) -> Image {
		return Image(width, height, new u8[u64(width) * height * 4]());
	}

	auto Image::makeEmpty() -> Image {
		return Image(0, 0, nullptr);
	}

	auto Image::resize(u32 width, u32 height) -> void {
		delete[] pixels;
		this->pixels = new u8[width * height * 4];

		this->width = width;
		this->height = height;
	}

	auto Image::getWidth() const -> u32 {
		return width;
	}

	auto Image::getHeight() const -> u32 {
		return height;
	}

	auto Image::getPixels() const -> u8* {
		return pixels;
	}

	auto Image::write(std::filesystem::path& path) const -> void {
		auto* file = static_cast<FILE*>(nullptr);
		
		_wfopen_s(&file, path.c_str(), L"wb");

		auto* png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		auto* info = png_create_info_struct(png);

		png_init_io(png, file);

		png_set_IHDR(
			png,
			info,
			width, height,
			8,
			PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT
		);

		png_write_info(png, info);

		auto* row = new u8[png_get_rowbytes(png, info)];
		
		/* convert image back into bytes */
		for (auto j = 0u; j < height; ++j) {
			for (auto i = 0u; i < width * 4; ++i)
				row[i]  = pixels[j * width * 4 + i];

			png_write_row(png, row);
		}
		
		delete[] row;
		
		png_write_end(png, nullptr);

		png_destroy_write_struct(&png, &info);

		fclose(file);
	}

	auto Image::isValid() -> bool {
		return pixels != nullptr;
	}

	auto Image::invalidate() -> void {
		delete[] pixels;
		pixels = nullptr;
	}
}
