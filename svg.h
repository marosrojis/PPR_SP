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

/*
	Trida slouzici pro generovani SVG souboru
*/
class SVG
{
public:
	SVG();
	~SVG();
	void print_graph(vector<point*> *values, vector<point*> *values_average, vector<segment_peaks*> &peaks, size_t peaks_start_index, size_t peaks_end_index, size_t index);
	void print_graph_split_segment(vector<point*> *points, vector<segment_points*> &points_by_day, size_t* point_position, vector<segment_peaks*> &peaks,
		size_t peaks_start_index, size_t peaks_end_index, size_t segmentid);
	static const int TIME_MARGIN = 20;
	static const int CENTER_TIME = 18;
	static const int SECOND_IN_MINUTE = 60;
	static const int COUNT_VALUES_Y_AXIS = 4;

private:
	void print_polynate_full_segment(tinyxml2::XMLPrinter* printer, vector<point*> &values, string color);
	void print_polynate_split_segment(tinyxml2::XMLPrinter* printer, vector<point*> &values, string color);
	void print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> &peaks, point* y_max, bool full_graph);
	void print_border(tinyxml2::XMLPrinter* printer);
	void print_axis(tinyxml2::XMLPrinter* printer, vector<point*> &values, point* x_max, point* y_max, point* y_min, bool full_graph);
	void print_title(tinyxml2::XMLPrinter* printer, size_t segmentid, point* x_max);
	void label_of_axis_full_segment(tinyxml2::XMLPrinter* printer, vector<point*> &values, point* x_max, point* y_max, point* y_min);
	void label_of_axis_split_segment(tinyxml2::XMLPrinter* printer, point* x_max, point* y_max, point* y_min);
	void create_g_transform(tinyxml2::XMLPrinter* printer, float y);
};

#endif
