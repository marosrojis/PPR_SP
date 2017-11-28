#pragma once
#ifndef __STRUCTS__
#define __STRUCTS__

#include <stdlib.h>
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#pragma warning( push )
#pragma warning( disable : 4996)
#include <CL/cl.hpp>
#pragma warning( pop )

typedef struct measured_value
{
	unsigned int id;
	unsigned int second;
	unsigned int second_of_day;
	float ist;
	size_t segmentid;
} MEASUREDVALUE;

typedef struct point
{
	float x;
	float y;
	unsigned int second;
	float ist;
} POINT;

typedef struct segment_points
{
	std::vector<point*> *points;
	size_t segmentid;
} SEGMENT_POINTS;

typedef struct peak
{
	point* x1;
	point* x2;
	float sum;
} PEAKS;

typedef struct segment_peaks
{
	std::vector<peak*> *peaks;
	size_t segmentid;
} SEGMENT_PEAKS;

typedef struct cl_config {
	cl_platform_id platform_id;
	cl_program program;
	cl_context context;
	cl_device_id device_id;
	cl_kernel kernel;
} CL_CONFIG;

typedef struct config
{
	bool run_serial = false;
	bool run_tbb = false;
	bool run_gpu = false;
	int number_of_threads = 4;
	bool split_segments = false;
	bool print_stats = false;
	bool valid_input = false;
	size_t number_of_start;
	const char* db_file_name;
} CONFIG;

#endif
