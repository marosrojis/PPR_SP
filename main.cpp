#include <ios>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <map>
#include <chrono>
#include "database.h"
#include "graph.h"
#include "svg.h"
#include "validator.h"
#include "free_function.h"
#include "tinyxml2.h"

#include "graph_opencl.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;
using namespace tinyxml2;

Database *db;
SVG *svg;

/*
	Funkce pro vytvoreni vsech SVG souboru, kde jeden soubor bude obsahovat jeden graf celeho segmentu

	points - vektor obsahujici vsechny segmenty a jejich body
	points_average - vektor obsahujici vsechny segmenty a body vytvorene pomoci klouzaveho prumeru
	peaks - vektor obsahujici vsechny vykyvy
	peak_segment_position - pole obsahujici indexy zacatku vykyvu segmentu
*/
void print_all_segments(vector<segment_points*> &points, vector<segment_points*> &points_average, vector<segment_peaks*> &peaks, size_t* peak_segment_position) {
	size_t peak_position = 0;
	for (size_t i = 0; i < points.size(); i++) {
		size_t peaks_start_index = peak_segment_position[i];
		size_t peaks_end_index = (i == points.size() - 1) ? peaks.size() : peak_segment_position[i + 1];
		svg->print_graph(points.at(i)->points, points_average.at(i)->points, peaks, peaks_start_index, peaks_end_index, points.at(i)->segmentid);
	}
}

/*
Funkce pro vytvoreni vsech SVG souboru, kde jeden soubor bude obsahovat vice grafu rozdelenych na jednotlive dny v ramci segmentu

points - vektor obsahujici vsechny segmenty a jejich body
points_average - vektor obsahujici vsechny segmenty a body vytvorene pomoci klouzaveho prumeru
peaks - vektor obsahujici vsechny vykyvy
peak_segment_position - pole obsahujici indexy zacatku vykyvu segmentu
*/
void print_all_split_segments(vector<segment_points*> &points, vector<segment_points*> &points_by_day, vector<segment_peaks*> &peaks, size_t* peak_segment_position) {
	size_t points_position = 0;
	for (size_t i = 0; i < points.size(); i++) {
		size_t peaks_start_index = peak_segment_position[i];
		size_t peaks_end_index = (i == points.size() - 1) ? peaks.size() : peak_segment_position[i + 1];
		svg->print_graph_split_segment(points.at(i)->points, points_by_day, &points_position, peaks, peaks_start_index, peaks_end_index, points.at(i)->segmentid);
	}
}

/*
	Funkce ktera vytiskne statistiku ve formatu CSV

	points - vektor obsahujici vsechny segmenty a jejich body
	peaks - vektor obsahujici vsechny vykyvy
	peak_segment_position - pole obsahujici indexy zacatku vykyvu segmentu
*/
void print_stats(vector<segment_points*> &points, vector<segment_peaks*> &peaks, size_t* peak_segment_position) {
	ostringstream ret_stream;
	ret_stream << "ID segment;Minimum ist value;Maximum ist value;Average ist value;Found peaks\n";

	for (size_t i = 0; i < points.size(); i++) {
		segment_points* segment = points.at(i);

		float average = 0, min_ist = segment->points->at(0)->ist, max_ist = 0;
		for (size_t y = 0; y < segment->points->size(); y++) {
			point* temp = segment->points->at(y);

			if (min_ist > temp->ist) {
				min_ist = temp->ist;
			}
			if (max_ist < temp->ist) {
				max_ist = temp->ist;
			}
			average += temp->ist;
		}
		average /= segment->points->size();

		size_t peaks_start_index = peak_segment_position[i];
		size_t peaks_end_index = (i == points.size() - 1) ? peaks.size() : peak_segment_position[i + 1];
		size_t count_peaks = 0;
		for (size_t y = peaks_start_index; y < peaks_end_index; y++) {
			count_peaks += peaks.at(y)->peaks->size();
		}

		ret_stream << segment->segmentid << ";" << min_ist << ";" << max_ist << ";" << average << ";" << count_peaks << "\n";
	}
	
	cout << ret_stream.str();

}

/*
	Funkce pro spusteni vsech vypoctu na zaklade zadanych parametru uzivatelem

	values - mapa obsahujici vsechny hodnoty measured_value ziskanych z 
	cfg - config pro spusteni vypoctu (vytvoren z parametru zadane uzivatelem)
*/
void get_calculate_point(map<size_t, vector<measured_value*>> &values, config* cfg) {
	vector<segment_points*> points_average;
	vector<segment_peaks*> peaks;
	map<size_t, vector<measured_value*>> values_average;
	
	map<size_t, float> max_values = get_max_values(values);
	vector<segment_points*> points = get_points_from_values(values, max_values, false);
	vector<segment_points*> points_by_day = split_segments_by_day(points);

	size_t* peak_segment_position = (size_t*)malloc(sizeof(size_t) * points.size());
	if (peak_segment_position == nullptr) {
		printf("Malloc memory error\n");
		return;
	}

	for (size_t i = 0; i < cfg->number_of_start; i++) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		if (cfg->run_serial) {
			values_average = calculate_moving_average(values);
			if (values_average.size() == 0) {
				return;
			}
			points_average = get_points_from_values(values_average, max_values, true);
			peaks = get_peaks(points, points_average, points_by_day, &peak_segment_position);
		}
		else if (cfg->run_tbb) {
			values_average = calculate_moving_average_tbb(values);
			if (values_average.size() == 0) {
				return;
			}
			points_average = get_points_from_values(values_average, max_values, true);
			peaks = get_peaks_tbb(points, points_average, points_by_day, &peak_segment_position);
		}
		else if (cfg->run_gpu) {
			cl_config* config = prepare_opencl_config();

			begin = std::chrono::steady_clock::now();
			values_average = calculate_moving_average(values);
			if (values_average.size() == 0) {
				return;
			}
			points_average = get_points_from_values(values_average, max_values, true);
			peaks = get_peaks_opencl(config, points, points_average, points_by_day, &peak_segment_position);

			free(config);
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		if (i + 1 < cfg->number_of_start) {
			free_points(points_average);
			free_map_measured_values(values_average);
			free_peaks(peaks);
		}
	}

	if (cfg->split_segments) {
		print_all_split_segments(points, points_by_day, peaks, peak_segment_position);
	}
	else {
		print_all_segments(points, points_average, peaks, peak_segment_position);
	}

	if (cfg->print_stats) {
		print_stats(points, peaks, peak_segment_position);
	}
	
	free_peaks(peaks);
	free_points(points);
	free_points(points_average);
	free_map_measured_values(values_average);
	free_segment_points(points_by_day);
	free(peak_segment_position);
}

/*
	Funkce pro rozdeleny vsech hodnot do mapy jednotlivych segmentu

	values - vsechny hodnoty measuredValue ziskane z DB
*/
map<size_t, vector<measured_value*>> transform_measured_value(vector<measured_value*> &values) {
	map<size_t, vector<measured_value*>> values_map;
	for (auto &row : values) {
		map<size_t, vector<measured_value*>>::iterator p = values_map.find(row->segmentid);
		if (p != values_map.end()) {
			p->second.push_back(row);
		}
		else {
			vector<measured_value*> temp;
			temp.push_back(row);
			values_map[row->segmentid] = temp;
		}
	}
	return values_map;
}

/*
	Funkce pro spusteni programu

	cfg - config pro spusteni vypoctu (vytvoren z parametru zadane uzivatelem)
*/
void start_program(config* cfg) {
	db = new Database(cfg->db_file_name);
	svg = new SVG();

	vector<measured_value*> values = db->get_measured_value();

	map<size_t, vector<measured_value*>> values_map = transform_measured_value(values);
	if (values_map.size() == 0) {
		return;
	}
	
	get_calculate_point(values_map, cfg);

	free_measured_values(values);
	delete(svg);
	delete(db);
}

/*
	Vychozi funkce, ktera se zavola po spusteni programu. Slouzi primarne k osetreni vstupnich parametru od uzivatele.
*/
void run(int argc, char *argv[]) {
	vector<string> args(argv + 1, argv + argc + !argc);
	config* cfg = validate_input(args);
	if (cfg->valid_input) {
		start_program(cfg);
	}
	free(cfg);
}

/*
	Hlavni funkce main. Vola se funkce run().
	Po odkomentovani _CrtDumpMemoryLeaks() je mozne zjistit pripadne memory leaky
*/
int main(int argc, char *argv[])
{
	run(argc, argv);

	//_CrtDumpMemoryLeaks();
	
	// Wait For User To Close Program
	/*cout << "Please press any key to exit the program ..." << endl;
	cin.get();*/

	return 0;

}

