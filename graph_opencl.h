#pragma once
#ifndef __GRAPH_OPENCL_H__
#define __GRAOH_OPENCL_H__

#include <vector>
#include <cmath>
#include <algorithm>
#include "structs.h"
#include "graph.h"
#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <CL/cl.hpp>
#pragma warning( pop )

#define MAX_SOURCE_SIZE (0x100000)

using namespace std;

vector<segment_peaks*> get_peaks_opencl(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** result_segments_position);

#endif