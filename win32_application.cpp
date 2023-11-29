global s64 global_perf_count_frequency;

inline s64
win32_get_wall_clock() {
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result.QuadPart;
}

// used to init global_perf_count_frequency
inline s64
win32_performance_frequency() {
	LARGE_INTEGER result;
	QueryPerformanceFrequency(&result);
	return result.QuadPart;
}

inline r64
win32_get_seconds_elapsed(s64 start, s64 end)
{
    r64 result = ((r64)(end - start) / (r64)global_perf_count_frequency);
    return result;
}

function void
win32_update_time(Time *time) {
	r64 last_run_time_ms = time->run_time_ms;
    
	local_persist s64 start = 0;
	s64 end = win32_get_wall_clock();
    time->frame_time_s = (r32)win32_get_seconds_elapsed(start, end);
    start = end;

    time->run_time_s += time->frame_time_s;

    time->frame_time_ms = time->frame_time_s * 1000.0f;
    time->run_time_ms += time->frame_time_ms;

    time->frames_per_s = (r32)(1.0 / time->frame_time_s);
}