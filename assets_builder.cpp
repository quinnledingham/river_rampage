#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "data_structures.h"

#include "log.cpp"
#include "data_structures.cpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include "assets.cpp"

void *platform_malloc(u32 size) {
    return SDL_malloc(size);
}

void platform_free(void *ptr) {
    SDL_free(ptr);
}

int main(int argc, char *argv[]) {
	Assets assets = {};

	if (argc != 2) {
		error(0, "usage: ./builder.exe <save_type>");
		return 1;
	}

	const char *save_type = argv[1];

	if (load_assets(&assets, "../assets.ethan")) {
		error(0, "load_assets() failed");
		return 1;
	}

	if (equal(save_type, "file")) {
		FILE *file = fopen("assets.save", "wb");
		save_assets(&assets, file);
		fclose(file);
	} else if (equal(save_type, "exe")) {
		save_assets_to_exe("river.exe", &assets);
	}

	return 0;
}