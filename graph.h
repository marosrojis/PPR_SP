#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <map>
#include <vector>
#include "structs.h"

#define MOVING_AVERAGE 15
#define Y_SCALE 30
#define MINUTE 60

using namespace std;

map<unsigned int, vector<peak*>> get_peaks(map<unsigned int, vector<point*>> points, map<unsigned int, vector<point*>> points_average);
map<unsigned int, float> get_max_values(map<unsigned int, vector<measuredValue*>> values);
map<unsigned int, vector<point*>> get_points_from_values(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, float> max_values);
map<unsigned int, vector<measuredValue*>> calculate_moving_average(map<unsigned int, vector<measuredValue*>> values_map);
float get_max_value_ist(vector<measuredValue*> values);

#endif
