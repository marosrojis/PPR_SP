#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iterator>
#include "structs.h"
#include "tbb/parallel_for.h"
#include "tbb/task_arena.h"


const int MOVING_AVERAGE = 21;
const int Y_SCALE = 15;
const int MINUTE = 60;
const int MIN_MINUTE_FOR_ACTION = 30;
const int SHOW_PEAKS = 3;


using namespace std;

segment_peaks* create_segment_peaks(vector<peak*> *peaks, size_t segmentid);
vector<segment_peaks*> get_peaks_tbb(vector<segment_points*> &points, vector<segment_points*> &points_average, vector<segment_points*> &points_by_day, size_t** peak_segment_position, config* cfg);
vector<segment_peaks*> get_peaks(vector<segment_points*> &points, vector<segment_points*> &points_average, vector<segment_points*> &points_by_day, size_t** peak_segment_position);
map<size_t, float> get_max_values(map<size_t, vector<measured_value*>> &values);
vector<segment_points*> split_segments_by_day(vector<segment_points*> &segments);
vector<segment_points*> get_points_from_values(map<size_t, vector<measured_value*>> &values, map<size_t, float> &max_values, bool isAverage);
map<size_t, vector<measured_value*>> calculate_moving_average_tbb(map<size_t, vector<measured_value*>> &values_map, config* cfg);
map<size_t, vector<measured_value*>> calculate_moving_average(map<size_t, vector<measured_value*>> &values_map);
float get_max_value_ist(vector<measured_value*> &values);

#endif
