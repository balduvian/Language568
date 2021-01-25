
#ifndef CNGE_IMAGE
#define CNGE_IMAGE

#include <filesystem>
#include <memory>

#include "types.h"

namespace CNGE {
	class Image {
	private:
		u32 width;
		u32 height;
		
		u8* pixels;
		
	public:
		static auto fromPNG(const char *) -> std::unique_ptr<Image>;

		static auto makeSheet(u32, u32) -> Image;
		static auto makeEmpty() -> Image;

		Image();
		Image(u32, u32, u8*);
		Image(Image&&);

		auto operator=(Image&&) -> Image&;
		~Image();

		auto resize(u32, u32) -> void;

		auto getWidth() const -> u32;
		auto getHeight() const -> u32;
		
		auto getPixels() const -> u8*;

		auto write(std::filesystem::path&) const -> void;

		auto isValid() -> bool;
		auto invalidate() -> void;
	};
}

#endif
