#pragma once
#ifndef __FREE_FUNCTION_H__
#define __FREE_FUNCTION_H__

#include <vector>
#include <map>
#include "structs.h"

using namespace std;

void free_measured_values(vector<measured_value*> values);
void free_map_measured_values(map<size_t, vector<measured_value*>> values);
void free_points(vector<segment_points*> points);
void free_segment_points(vector<segment_points*> points);
void free_peaks(vector<segment_peaks*> peaks);

#endif