#pragma once
#ifndef __GRAPH_OPENCL_H__
#define __GRAOH_OPENCL_H__

#include <vector>
#include <cmath>
#include <algorithm>
#include "structs.h"
#include "opencl_main.h"
#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <CL/cl.hpp>
#pragma warning( pop )

#define MOVING_AVERAGE 21
#define MIN_MINUTE_FOR_ACTION 30
#define SHOW_PEAKS 6
#define MAX_SOURCE_SIZE (0x100000)

using namespace std;

vector<segment_peaks*> get_peaks_opencl(vector<segment_points*> points, vector<segment_points*> points_average);

#endif