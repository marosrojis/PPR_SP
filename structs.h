#pragma once
#ifndef __STRUCTS__
#define __STRUCTS__

#include <stdlib.h>

typedef struct measuredValue
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
