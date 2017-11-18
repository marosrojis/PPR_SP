#pragma once
#ifndef __COLUMNS_TYPE__
#define __COLUMNS_TYPE__

#include <stdlib.h>

typedef struct measuredValue
{
	unsigned int id;
	int64_t day;
	unsigned int second;
	unsigned int second_of_day;
	float ist;
	int segmentid;
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
	int segmentid;
} SEGMENT_POINTS;

typedef struct peak
{
	float x1;
	float x2;
	float sum;
} PEAKS;

typedef struct segment_peaks
{
	std::vector<peak*> *peaks;
	int segmentid;
} SEGMENT_PEAKS;

#endif
