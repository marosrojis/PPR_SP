#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include "structs.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"


#define MOVING_AVERAGE 21
#define Y_SCALE 30
#define MINUTE 60
#define MIN_SECOND_FOR_ACTION 30
#define SHOW_PEAKS 6

using namespace std;

vector<segment_peaks*> get_peaks(vector<segment_points*> points, vector<segment_points*> points_average);
map<unsigned int, float> get_max_values(map<unsigned int, vector<measuredValue*>> values);
vector<segment_points*> get_points_from_values(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, float> max_values, bool isAverage);
map<unsigned int, vector<measuredValue*>> calculate_moving_average(map<unsigned int, vector<measuredValue*>> values_map);
float get_max_value_ist(vector<measuredValue*> values);

#endif
