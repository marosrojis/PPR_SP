#pragma once
#ifndef __GRAPH_OPENCL_H__
#define __GRAOH_OPENCL_H__

#include <vector>
#include <cmath>
#include <algorithm>
#include "structs.h"
#include "graph.h"
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#pragma warning( push )
#pragma warning( disable : 4996)
#include <CL/cl.hpp>
#pragma warning( pop )

#define MAX_SOURCE_SIZE (0x100000)

using namespace std;

cl_config* prepare_opencl_config();
vector<segment_peaks*> get_peaks_opencl(cl_config* config, vector<segment_points*> &points, vector<segment_points*> &points_average, vector<segment_points*> &points_by_day, size_t** result_segments_position);

#endif