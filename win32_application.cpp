global s64 global_perf_count_frequency;

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