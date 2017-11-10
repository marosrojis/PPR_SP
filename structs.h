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
	unsigned int ist;
} POINT;

typedef struct peak
{
	float x1;
	float x2;
	float y1;
	float y2;
} PEAKS;

#endif
