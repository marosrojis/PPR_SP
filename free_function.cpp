#include "free_function.h"

/*
	Uvoleneni alokovane pameti pro struktury measured_value
*/
void free_measured_values(vector<measured_value*> values) {
	for (auto &row : values) {
		free(row);
	}
}

/*
	Uvoleneni alokovane pameti pro struktury measured_value
*/
void free_map_measured_values(map<size_t, vector<measured_value*>> values) {
	for (auto &segment : values) {
		for (auto &value : segment.second) {
			free(value);
		}
	}
}

/*
	Uvolneni pameti pro vsechny alokovane segmenty + vsechny body v ramci segmentu
*/
void free_points(vector<segment_points*> points) {
	for (auto &segment : points) {
		for (auto &point : *(segment->points)) {
			free(point);
		}
		delete(segment->points);
		free(segment);
	}
}

/*
	Uvolneni pameti pro vsechny alokovane segmenty + vektor pro body
*/
void free_segment_points(vector<segment_points*> points) {
	for (auto &segment : points) {
		delete(segment->points);
		free(segment);
	}
}

/*
	Uvolneni pameti pro vsechny nalezene vykyvy
*/
void free_peaks(vector<segment_peaks*> peaks) {
	for (auto &segment : peaks) {
		for (auto &peak : *(segment->peaks)) {
			free(peak);
		}
		delete(segment->peaks);
		free(segment);
	}
}