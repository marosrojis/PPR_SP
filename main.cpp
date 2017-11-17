#include <ios>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <map>
#include <chrono>
#include "database.h"
#include "graph.h"
#include "svg.h"
#include "opencl_main.h"
#include "tinyxml2.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;
using namespace tinyxml2;

Database *db;
SVG *svg;

void freeMeasuredValues(vector<measuredValue*> values) {
	for (auto &row : values) {
		free(row);
	}
}

void freeMapMeasuredValues(map<unsigned int, vector<measuredValue*>> values) {
	for (auto &segment : values) {
		for (auto &value : segment.second) {
			free(value);
		}
	}
}

void freePoints(vector<segment_points*> points) {
	for (auto &segment : points) {
		for (auto &point : *(segment->points)) {
			free(point);
		}
		delete(segment->points);
		free(segment);
	}
}

void freePeaks(vector<segment_peaks*> peaks) {
	for (auto &segment : peaks) {
		for (auto &peak : *(segment->peaks)) {
			free(peak);
		}
		delete(segment->peaks);
		free(segment);
	}
}

void printAllSegments(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, vector<measuredValue*>> values_average) {
	map<unsigned int, float> max_values = get_max_values(values);
	vector<segment_points*> points = get_points_from_values(values, max_values, false);
	vector<segment_points*> points_average = get_points_from_values(values_average, max_values, true);
	vector<segment_peaks*> peaks = get_peaks(points, points_average);

	/*for (size_t i = 0; i < points.size(); i++) {
		//svg->print_graph(row.second, points_average.find(row.first)->second, peaks.find(row.first)->second, row.first);
		svg->print_graph(points.at(i)->points, points_average.at(i)->points, peaks.at(i)->peaks, i);
	}*/

	freePoints(points);
	freePoints(points_average);
	freePeaks(peaks);
}

map<unsigned int, vector<measuredValue*>> transform_measured_value(vector<measuredValue*> values) {
	map<unsigned int, vector<measuredValue*>> values_map;
	for (auto &row : values) {
		map<unsigned int, vector<measuredValue*>>::iterator p = values_map.find(row->segmentid);
		if (p != values_map.end()) {
			p->second.push_back(row);
		}
		else {
			vector<measuredValue*> temp;
			temp.push_back(row);
			values_map[row->segmentid] = temp;
		}
	}
	return values_map;
}

void run() {
	db = new Database();
	svg = new SVG();

	vector<measuredValue*> values = db->get_measured_value();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	map<unsigned int, vector<measuredValue*>> values_map = transform_measured_value(values);
	if (values_map.size() == 0) {
		return;
	}

	map<unsigned int, vector<measuredValue*>> values_average = calculate_moving_average(values_map);
	if (values_average.size() == 0) {
		return;
	}
	
	printAllSegments(values_map, values_average);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;

	freeMeasuredValues(values);
	freeMapMeasuredValues(values_average);
	delete(svg);
	delete(db);
}

int main()
{
	//run();

	opencl_main_test();

	//_CrtDumpMemoryLeaks();
	
	// Wait For User To Close Program
	cout << "Please press any key to exit the program ..." << endl;
	cin.get();

	return 0;

}

