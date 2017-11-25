#include <ios>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <map>
#include <chrono>
#include "database.h"
#include "graph.h"
#include "svg.h"
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

void freeMapMeasuredValues(map<size_t, vector<measuredValue*>> values) {
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
		size_t peaks_end_index = (i == points.size() - 1) ? peaks.size() : peak_segment_position[i + 1];
		svg->print_graph(points.at(i)->points, points_average.at(i)->points, peaks, peaks_start_index, peaks_end_index, points.at(i)->segmentid);
	}
}

void printAllSplitSegments(vector<segment_points*> points, vector<segment_points*> points_by_day, vector<segment_peaks*> peaks, size_t* peak_segment_position) {
	size_t points_position = 0;
	for (size_t i = 0; i < points.size(); i++) {
		size_t peaks_start_index = peak_segment_position[i];
		size_t peaks_end_index = (i == points.size() - 1) ? peaks.size() : peak_segment_position[i + 1];
		svg->print_graph_split_segment(points.at(i)->points, points_by_day, &points_position, peaks, peaks_start_index, peaks_end_index, points.at(i)->segmentid);
	}
}

void get_calculate_point(map<size_t, vector<measuredValue*>> values, vector<string> args) {
	vector<segment_points*> points_average;
	vector<segment_peaks*> peaks;
	map<size_t, vector<measuredValue*>> values_average;
	
	map<size_t, float> max_values = get_max_values(values);
	vector<segment_points*> points = get_points_from_values(values, max_values, false);
	vector<segment_points*> points_by_day = split_segments_by_day(points);

	size_t* peak_segment_position = (size_t*)malloc(sizeof(size_t) * points.size());
	if (peak_segment_position == nullptr) {
		printf("Malloc memory error\n");
		return;
	}

	
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	if (find(args.begin(), args.end(), "-serial") != args.end()) {
		values_average = calculate_moving_average(values);
		if (values_average.size() == 0) {
			return;
		}
		points_average = get_points_from_values(values_average, max_values, true);
		peaks = get_peaks(points, points_average, points_by_day, &peak_segment_position);
	}
	else if (find(args.begin(), args.end(), "-tbb") != args.end()) {
		values_average = calculate_moving_average_tbb(values);
		if (values_average.size() == 0) {
			return;
		}
		points_average = get_points_from_values(values_average, max_values, true);
		peaks = get_peaks_tbb(points, points_average, points_by_day, &peak_segment_position);
	}
	else if (find(args.begin(), args.end(), "-gpu") != args.end()) {
		values_average = calculate_moving_average(values);
		if (values_average.size() == 0) {
			return;
		}
		points_average = get_points_from_values(values_average, max_values, true);
		peaks = get_peaks_opencl(points, points_average, points_by_day, &peak_segment_position);
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;

	if (find(args.begin(), args.end(), "-d") != args.end()) {
		printAllSplitSegments(points, points_by_day, peaks, peak_segment_position);
	}
	else {
		printAllSegments(points, points_average, peaks, peak_segment_position);
	}
	
	freePeaks(peaks);
	freePoints(points);
	freePoints(points_average);
	freeMapMeasuredValues(values_average);
	freeSegmentPoints(points_by_day);
	free(peak_segment_position);
}

map<size_t, vector<measuredValue*>> transform_measured_value(vector<measuredValue*> values) {
	map<size_t, vector<measuredValue*>> values_map;
	for (auto &row : values) {
		map<size_t, vector<measuredValue*>>::iterator p = values_map.find(row->segmentid);
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

void startProgram(vector<string> args) {
	db = new Database();
	svg = new SVG();

	vector<measuredValue*> values = db->get_measured_value();

	map<size_t, vector<measuredValue*>> values_map = transform_measured_value(values);
	if (values_map.size() == 0) {
		return;
	}
	
	get_calculate_point(values_map, args);

	freeMeasuredValues(values);
	delete(svg);
	delete(db);
}

void printHelp() {
	ostringstream retStream;

	retStream << "Usage: PPR.exe [-h] [-serial | -tbb | -gpu] [-stats] [-d]\n\n";
	retStream << "Options:\n";
	retStream << "\t-d\t\t Split segment on days in graph.\n";
	retStream << "\t-h\t\t Show help.\n";
	retStream << "\t-gpu\t\t Start parallel version using GPU.\n";
	retStream << "\t-serial\t\t Start serial version.\n";
	retStream << "\t-tbb\t\t Start parallel version using Threading Building Blocks.\n\n";

	cout << retStream.str();
}

bool validateInput(vector<string> args) {
	for (auto &input : args) {
		if (!(input == "-h" || input == "-serial" || input == "-tbb" || input == "-gpu" || input == "-stats" || input == "-d")) {
			cout << "Invalid argument \""<< input << ".\"\n" << endl;
			printHelp();
			return false;
		}
	}

	if (args.size() == 0 || find(args.begin(), args.end(), "-h") != args.end()) {
		printHelp();
		return false;
	}

	if (!(find(args.begin(), args.end(), "-serial") != args.end() || find(args.begin(), args.end(), "-tbb") != args.end() ||
		find(args.begin(), args.end(), "-gpu") != args.end())) {
		cout << "Select version of program (serial, tbb or gpu).\n\n" << endl;
		printHelp();
		return false;
	}

	if ((find(args.begin(), args.end(), "-serial") != args.end() && find(args.begin(), args.end(), "-tbb") != args.end()) || 
		(find(args.begin(), args.end(), "-serial") != args.end() && find(args.begin(), args.end(), "-gpu") != args.end()) ||
		find(args.begin(), args.end(), "-gpu") != args.end() && find(args.begin(), args.end(), "-tbb") != args.end()) {
		cout << "Use only one version of program!\n\n" << endl;
		printHelp();
		return false;
	}

	return true;
}

void run(int argc, char *argv[]) {
	vector<string> args(argv + 1, argv + argc + !argc);
	if (validateInput(args)) {
		startProgram(args);
	}
}

int main(int argc, char *argv[])
{
	run(argc, argv);

	_CrtDumpMemoryLeaks();
	
	// Wait For User To Close Program
	/*cout << "Please press any key to exit the program ..." << endl;
	cin.get();*/

	return 0;

}

