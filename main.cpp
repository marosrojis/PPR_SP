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

#include "graph_opencl.h"

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

void freeSegmentPoints(vector<segment_points*> points) {
	for (auto &segment : points) {
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

void printAllSegments(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_peaks*> peaks, size_t* peak_segment_position) {
	size_t peak_position = 0;
	for (size_t i = 0; i < points.size(); i++) {
		size_t peaks_start_index = peak_segment_position[i];
		size_t peaks_end_index = (i == points.size() - 1) ? peaks.size() - 1 : peak_segment_position[i + 1];
		svg->print_graph(points.at(i)->points, points_average.at(i)->points, peaks, peaks_start_index, peaks_end_index, i);
	}
}

void printAllSplitSegments(vector<segment_points*> points, vector<segment_points*> points_by_day, vector<segment_peaks*> peaks) {
	size_t peak_position = 0, points_position = 0;
	for (size_t i = 0; i < points.size(); i++) {
		svg->print_graph_split_segment(points.at(i)->points, points_by_day, &points_position, peaks, &peak_position, points.at(i)->segmentid);
	}
}

void get_calculate_point(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, vector<measuredValue*>> values_average) {
	bool splitSegment = false;
	
	map<unsigned int, float> max_values = get_max_values(values);
	vector<segment_points*> points = get_points_from_values(values, max_values, false);
	vector<segment_points*> points_average = get_points_from_values(values_average, max_values, true);
	vector<segment_points*> points_by_day = split_segments_by_day(points);

	size_t* peak_segment_position = (size_t*)malloc(sizeof(size_t) * points.size());

	vector<segment_peaks*> peaks = get_peaks(points, points_average, points_by_day, &peak_segment_position);

	if (splitSegment) {
		printAllSplitSegments(points, points_by_day, peaks);
	}
	else {
		printAllSegments(points, points_average, peaks, peak_segment_position);
	}
	
	freePoints(points);
	freePoints(points_average);
	freePeaks(peaks);
	freeSegmentPoints(points_by_day);
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

	map<unsigned int, vector<measuredValue*>> values_average = calculate_moving_average_tbb(values_map);
	if (values_average.size() == 0) {
		return;
	}
	
	get_calculate_point(values_map, values_average);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;

	freeMeasuredValues(values);
	freeMapMeasuredValues(values_average);
	delete(svg);
	delete(db);
}

int main()
{
	run();

	_CrtDumpMemoryLeaks();
	
	// Wait For User To Close Program
	cout << "Please press any key to exit the program ..." << endl;
	cin.get();

	return 0;

}

