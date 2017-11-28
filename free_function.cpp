#include "free_function.h"

void free_measured_values(vector<measured_value*> values) {
	for (auto &row : values) {
		free(row);
	}
}

void free_map_measured_values(map<size_t, vector<measured_value*>> values) {
	for (auto &segment : values) {
		for (auto &value : segment.second) {
			free(value);
		}
	}
}

void free_points(vector<segment_points*> points) {
	for (auto &segment : points) {
		for (auto &point : *(segment->points)) {
			free(point);
		}
		delete(segment->points);
		free(segment);
	}
}

void free_segment_points(vector<segment_points*> points) {
	for (auto &segment : points) {
		delete(segment->points);
		free(segment);
	}
}

void free_peaks(vector<segment_peaks*> peaks) {
	for (auto &segment : peaks) {
		for (auto &peak : *(segment->peaks)) {
			free(peak);
		}
		delete(segment->peaks);
		free(segment);
	}
}