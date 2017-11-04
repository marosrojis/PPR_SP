#pragma once
#ifndef __SVG_H__
#define __SVG_H__

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "structs.h"
#include "tinyxml2.h"

using namespace std;

class SVG
{
public:
	SVG();
	~SVG();
	void print_graph(vector<point*> values, vector<point*> values_average, vector<peak*> peaks, int segmentid);
	void print_graph(vector<point*> values, vector<point*> values_average, vector<peak*> peaks, int segmentid, string color1, string color2);

private:
	void print_polynate(tinyxml2::XMLPrinter* printer, vector<point*> values, string color);
	void print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> peaks);
	void print_border(tinyxml2::XMLPrinter* printer);
	float get_max_x_point(vector<point*> values);
	float get_max_y_point(vector<point*> values);
};

#endif
