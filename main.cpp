#include <ios>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <map>
#include "database.h"
#include "graph.h"
#include "svg.h"
#include "tinyxml2.h"

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

void freePoints(map<unsigned int, vector<point*>> points) {
	for (auto &segment : points) {
		for (auto &point : segment.second) {
			free(point);
		}
	}
}

void printAllSegments(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, vector<measuredValue*>> values_average) {
	map<unsigned int, float> max_values = get_max_values(values);
	map<unsigned int, vector<point*>> points = get_points_from_values(values, max_values);
	map<unsigned int, vector<point*>> points_average = get_points_from_values(values_average, max_values);
	map<unsigned int, vector<peak*>> peaks = get_peaks(points, points_average);
	for (auto &row : points) {
		svg->print_graph(row.second, points_average.find(row.first)->second, peaks.find(row.first)->second, row.first);
	}

	freePoints(points);
	freePoints(points_average);
}

map<unsigned int, vector<measuredValue*>> transform_measured_value(vector<measuredValue*> values) {
	map<unsigned int, vector<measuredValue*>> values_map;
	for (auto &row : values) {
		if (row->ist == NULL) {
			continue;
		}
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

int main()
{
	db = new Database();
	svg = new SVG();
	vector<int> segmentsId = db->get_all_segments_id();

	vector<measuredValue*> values = db->get_measured_value();
	map<unsigned int, vector<measuredValue*>> values_map = transform_measured_value(values);
	map<unsigned int, vector<measuredValue*>> values_average = calculate_moving_average(values_map);
	printAllSegments(values_map, values_average);

	freeMeasuredValues(values);
	delete(svg);
	delete(db);

	// Wait For User To Close Program
	/*cout << "Please press any key to exit the program ..." << endl;
	cin.get();*/

	return 0;

}

