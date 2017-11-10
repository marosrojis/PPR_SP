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
	void print_graph(vector<point*> values, vector<point*> values_average, vector<peak*> peaks, int segmentid);
	void print_graph(vector<point*> values, vector<point*> values_average, vector<peak*> peaks, int segmentid, string color1, string color2);

private:
	void print_polynate(tinyxml2::XMLPrinter* printer, vector<point*> values, string color);
	void print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> peaks, float average);
	void print_border(tinyxml2::XMLPrinter* printer);
	void print_axis(tinyxml2::XMLPrinter* printer, vector<point*> values, point* x_max, point* y_max);
	float print_average_line(tinyxml2::XMLPrinter* printer, vector<point*> points);
};

#endif
