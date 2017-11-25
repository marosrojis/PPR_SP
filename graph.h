#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include "structs.h"
#include "tbb/parallel_for.h"


#define MOVING_AVERAGE 21
#define Y_SCALE 15
#define MINUTE 60
#define MIN_MINUTE_FOR_ACTION 30
#define SHOW_PEAKS 3

using namespace std;

segment_peaks* create_segment_peaks(vector<peak*> *peaks, size_t segmentid);
vector<segment_peaks*> get_peaks_tbb(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** peak_segment_position);
vector<segment_peaks*> get_peaks(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** peak_segment_position);
map<size_t, float> get_max_values(map<size_t, vector<measuredValue*>> values);
vector<segment_points*> split_segments_by_day(vector<segment_points*> segments);
vector<segment_points*> get_points_from_values(map<size_t, vector<measuredValue*>> values, map<size_t, float> max_values, bool isAverage);
map<size_t, vector<measuredValue*>> calculate_moving_average_tbb(map<size_t, vector<measuredValue*>> values_map);
map<size_t, vector<measuredValue*>> calculate_moving_average(map<size_t, vector<measuredValue*>> values_map);
float get_max_value_ist(vector<measuredValue*> values);

#endif
