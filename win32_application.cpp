#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include <wingdi.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "dx12/d3dx12.h" // Helper Structures and Functions

#include <string>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <shellapi.h>

#include <mmsystem.h>
#include <dsound.h>
#include <intrin.h>
#include <xinput.h>

// Enabling Dedicated Graphics on Laptops
// https://www.reddit.com/r/gamedev/comments/bk7xbe/psa_for_anyone_developing_a_gameengine_in_c/
//
// NOTE: If the laptop is in power saving mode (like it usually is when not plugged in) the graphics card
// will be severely limited. 
//
extern "C" 
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; // Nvidia
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;   // AMD
}

#include "log.h"
#include "types.h"

void *platform_malloc(u32 size) { return malloc(size); }
void platform_free(void *ptr)   { free(ptr); }
void platform_memory_copy(void *dest, void *src, u32 num_of_bytes) { memcpy(dest, src, num_of_bytes); }
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes) { memset(dest, value, num_of_bytes); }

#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "renderer.h"
#include "dx12_renderer.h"
#include "data_structures.h"
//#include "shapes.h"
//#include "particles.h"
#include "platform.h"

#include "log.cpp"
#include "data_structures.cpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include "assets.cpp"
#include "dx12_renderer.cpp"
//#include "renderer.cpp"
//#include "shapes.cpp"
//#include "particles.cpp"

//
// https://learn.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages
//
internal void
win32_print_error(DWORD err) {
	WCHAR buffer[512];  
    DWORD chars; 

    chars = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, buffer, 512, NULL);

	// Display the error message, or generic text if not found.
    printf("Error value: %d Message: %ws\n", err, chars ? buffer : L"Error message not found." );
}

// query performance counter has a high resolution so it can be used to get the micro seconds between frames.
// SDL_GetTicks only returns the microseconds.
inline s64
win32_get_ticks() {
	LARGE_INTEGER result;
	if (!QueryPerformanceCounter(&result)) {
		win32_print_error(GetLastError());
	}
	return result.QuadPart;
}

// used to init global_perf_count_frequency
inline s64
win32_performance_frequency() {
	LARGE_INTEGER result;
	if (!QueryPerformanceFrequency(&result)) {
		win32_print_error(GetLastError());
	}
	return result.QuadPart;
}

inline r64
win32_get_seconds_elapsed(s64 start, s64 end) {
    r64 result = ((r64)(end - start) / (r64)global_perf_count_frequency);
    return result;
}

function void
win32_update_time(Time *time) {
	s64 ticks = win32_get_ticks();

	// s
	local_persist s64 last_ticks = time->start; // time of last frame
    time->frame_time_s = (r32)win32_get_seconds_elapsed(last_ticks, ticks);
    last_ticks = ticks; // set last ticks for next frame

    // time->start has to be initialized before
    time->run_time_s = (r32)win32_get_seconds_elapsed(time->start, ticks);

    // ms
    time->frame_time_ms = time->frame_time_s * 1000.0f;
    time->run_time_ms   = time->run_time_s   * 1000.0f;

    // fps
    time->frames_per_s = (r32)(1.0 / time->frame_time_s);
}

dx_hello_triangle global_triangle = {};

LRESULT CALLBACK main_window_callback(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;

	switch(message) {
		case WM_SIZE: OutputDebugStringA("WM_SIZE\n"); break;

		case WM_CREATE: {

		} break;

		case WM_PAINT: {
			if (global_triangle.initialized)
				dx_on_render(&global_triangle);
		} break;

		case WM_DESTROY: {
			PostQuitMessage(0);
			result = 0;
		} break;

		default: {
			result = DefWindowProc(window_handle, message, wparam, lparam);
		} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR ld_cmdZ_line, int n_show_cmd) {
	WNDCLASS window_class = {};
	window_class.lpfnWndProc = main_window_callback;
	window_class.hInstance = h_instance;
	window_class.lpszClassName = "RiverRampageWindowClass";

	if (RegisterClassA(&window_class)) {
		HWND window_handle = CreateWindowExA(
			0, 
			window_class.lpszClassName,
			"River Rampage",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
            CW_USEDEFAULT,
            800,
            800,
            0,
            0,
            h_instance,
            0);

		if (window_handle) {

			v2s dim;
			RECT client_rect;
    		GetClientRect(window_handle, &client_rect);
			dim.width = client_rect.right - client_rect.left;
    		dim.height = client_rect.bottom - client_rect.top;

			init_hello_triangle(&global_triangle, dim.width, dim.height);
			dx_load_pipeline(&global_triangle, window_handle);
			dx_load_assets(&global_triangle);
			global_triangle.initialized = true;

			Application app = {};
			global_perf_count_frequency = win32_performance_frequency();
			app.time.start = win32_get_ticks();

			while(1) {
				MSG message;
				BOOL message_result = GetMessageA(&message, 0, 0, 0);
				if (message_result > 0) {
					TranslateMessage(&message);
					DispatchMessageA(&message);
				} else {
					break;
				}

				
				win32_update_time(&app.time);
				output("%f", app.time.frames_per_s);
			}
		} else {
			output("WinMain(): CreateWindowExA() failed");
		}
	} else {
		output("WinMain(): RegisterClassA() failed");
	}

	return 0;
}