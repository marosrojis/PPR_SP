#pragma once
#ifndef __OPENCL_MAIN_H__
#define __OPENCL_MAIN_H__

#include <stdlib.h>
#include <stdio.h>
#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <CL/cl.hpp>
#pragma warning( pop )

int get_peaks_opencl(int count_point_values, int count_point_average_values, int *count_segments, int* segment_positions, float* p_x, float* p_y, float* p_ist, float* pa_x, float* pa_y,
	int* result_count_peaks, float* peak_x1, float* peak_x2, float* peak_sum);
int opencl_main_test();


#endif