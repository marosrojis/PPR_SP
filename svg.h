#pragma once
#ifndef __SVG_H__
#define __SVG_H__

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "structs.h"
#include "utils.h"
#include "tinyxml2.h"

using namespace std;

class SVG
{
public:
	SVG();
	~SVG();
	void print_graph(vector<point*> *values, vector<point*> *values_average, vector<segment_peaks*> peaks, size_t peaks_start_index, size_t peaks_end_index, size_t index);
	void print_graph_split_segment(vector<point*> *points, vector<segment_points*> points_by_day, size_t* point_position, vector<segment_peaks*> peaks, size_t* peak_position, size_t segmentid);

private:
	void print_polynate(tinyxml2::XMLPrinter* printer, vector<point*> values, string color);
	void print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> peaks);
	void print_border(tinyxml2::XMLPrinter* printer);
	void print_axis(tinyxml2::XMLPrinter* printer, vector<point*> values, point* x_max, point* y_max);
};

#endif
