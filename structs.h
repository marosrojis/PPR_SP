#pragma once
#ifndef __COLUMNS_TYPE__
#define __COLUMNS_TYPE__

#include <stdlib.h>

typedef struct measuredValue
{
	unsigned int id;
	unsigned int day;
	unsigned int second;
	float ist;
	int segmentid;
} MEASUREDVALUE;

typedef struct point
{
	float x;
	float y;
} POINT;

typedef struct peak
{
	float x1;
	float x2;
} PEAKS;

#endif
