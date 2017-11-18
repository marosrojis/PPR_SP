#include "graph.h"

using namespace std;

bool compareBySum(const peak* a, const peak* b)
{
	return a->sum > b->sum;
}

segment_peaks** parallel_get_peaks(vector<segment_points*> points, vector<segment_points*> points_average, size_t size) {
	segment_peaks** data = (segment_peaks**)malloc(sizeof(segment_peaks*) * size);

	tbb::parallel_for(size_t(0), size, [&](size_t a) {
		vector<point*> segment = *(points.at(a)->points);

		bool is_peak = false;
		float x_peak = 0, y_peak = 0;
		float sum = 0, grow = 0;
		point* temp_peak = nullptr;
		size_t i = 0;
		vector<peak*> peaks;
		for (auto &point_base_line : segment) {
			if (i >= points_average.at(a)->points->size()) { // i je >= nez celkova velikost average vectoru, tak konec cyklu (uz neni kam sahat)
				break;
			}

			point* average_line = points_average.at(a)->points->at(i);

			if (point_base_line->x != average_line->x) { // tato podminka je kvuli zacatku points vektoru, protoze obsahuje body, ktery points_average neobsahuje
				continue;
			}

			if (point_base_line->y <= average_line->y) {
				if (!is_peak) {
					temp_peak = point_base_line;
					sum = 0;
					grow = 0;
					is_peak = true;
				}
				else {
					sum += abs(temp_peak->ist - point_base_line->ist);
					grow += temp_peak->ist - point_base_line->ist;
				}
			}
			else {
				if (is_peak) {
					if (point_base_line->x - x_peak >= MIN_MINUTE_FOR_ACTION && grow < 3) {
						peak* temp = (peak*)malloc(sizeof(peak));
						if (temp == NULL) {
							//TODO
						}

						temp->x1 = temp_peak->x;
						temp->x2 = point_base_line->x;
						temp->sum = sum;
						peaks.push_back(temp);

					}
					is_peak = false;
				}
			}
			i++;
		}
		sort(peaks.begin(), peaks.end(), compareBySum);

		size_t peak_size = peaks.size() >= SHOW_PEAKS ? SHOW_PEAKS : peaks.size();
		vector<peak*> *sort_peaks = new vector<peak*>(peak_size);
		for (size_t i = 0; i < peaks.size(); i++) {
			peak* tmp = peaks.at(i);
			if (i < peak_size) {
				(*sort_peaks)[i] = tmp;
			}
			else {
				free(tmp);
			}
		}

		segment_peaks* seg_peaks = (segment_peaks*)malloc(sizeof(segment_peaks));
		seg_peaks->peaks = sort_peaks;
		seg_peaks->segmentid = points.at(a)->segmentid;

		data[a] = seg_peaks;
	});

	return data;
}

vector<segment_peaks*> get_peaks_tbb(vector<segment_points*> points, vector<segment_points*> points_average) {
	segment_peaks** data = parallel_get_peaks(points, points_average, points.size());
	vector<segment_peaks*> results(data, data + points.size());

	free(data);
	return results;
}

vector<segment_peaks*> get_peaks(vector<segment_points*> points, vector<segment_points*> points_average) {
	vector<segment_peaks*> results;

	for (size_t a = 0; a < points.size(); a++) {
		vector<point*> segment = *(points.at(a)->points);

		bool is_peak = false;
		float x_peak = 0, y_peak = 0;
		float sum = 0, grow = 0;
		point* temp_peak = nullptr;
		size_t i = 0;
		vector<peak*> peaks;
		for (auto &point_base_line : segment) {
			if (i >= points_average.at(a)->points->size()) { // i je >= nez celkova velikost average vectoru, tak konec cyklu (uz neni kam sahat)
				break;
			}

			point* average_line = points_average.at(a)->points->at(i);
			if (point_base_line->x != average_line->x) { // tato podminka je kvuli zacatku points vektoru, protoze obsahuje body, ktery points_average neobsahuje
				continue;
			}

			if (point_base_line->y <= average_line->y) {
				if (!is_peak) {
					temp_peak = point_base_line;
					sum = 0;
					grow = 0;
					is_peak = true;
				}
				else {
					sum += abs(temp_peak->ist - point_base_line->ist);
					grow += temp_peak->ist - point_base_line->ist;
				}
			}
			else {
				if (is_peak) {
					if (point_base_line->x - x_peak >= MIN_MINUTE_FOR_ACTION && grow < 3) {
						peak* temp = (peak*)malloc(sizeof(peak));
						if (temp == NULL) {
							//TODO
						}

						temp->x1 = temp_peak->x;
						temp->x2 = point_base_line->x;
						temp->sum = sum;
						peaks.push_back(temp);

					}
					is_peak = false;
				}
			}
			i++;
		}
		sort(peaks.begin(), peaks.end(), compareBySum);

		size_t peak_size = peaks.size() >= SHOW_PEAKS ? SHOW_PEAKS : peaks.size();
		vector<peak*> *sort_peaks = new vector<peak*>(peak_size);
		for (size_t i = 0; i < peaks.size(); i++) {
			peak* tmp = peaks.at(i);
			if (i < peak_size) {
				(*sort_peaks)[i] = tmp;
			}
			else {
				free(tmp);
			}
		}

		segment_peaks* seg_peaks = (segment_peaks*)malloc(sizeof(segment_peaks));
		seg_peaks->peaks = sort_peaks;
		seg_peaks->segmentid = points.at(a)->segmentid;

		results.push_back(seg_peaks);
	}
	return results;
}

map<unsigned int, float> get_max_values(map<unsigned int, vector<measuredValue*>> values) {
	map<unsigned int, float> results;
	for (auto &value : values) {
		float max_value = get_max_value_ist(value.second);
		results[value.first] = max_value;
	}

	return results;
}

vector<segment_points*> get_points_from_values(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, float> max_values, bool isAverage) {
	vector<segment_points*> results;
	for (auto &row : values) {
		size_t i = 0;
		size_t vectorSize = (isAverage ? row.second.size() - MOVING_AVERAGE + 1: row.second.size());

		vector<point*> *points = new vector<point*>(vectorSize);

		unsigned int lastSecond = row.second.at(0)->second;
		for (auto &value : row.second) {
			if (value->ist != NULL) {
				point* temp = (point *)malloc(sizeof(point));
				if (temp == NULL) {
					//TODO
				}
				temp->x = static_cast<float>((value->second - lastSecond) / MINUTE);
				temp->y = (max_values.find(row.first)->second - value->ist) * Y_SCALE;
				temp->second = value->second_of_day;
				temp->ist = value->ist;
				(*points)[i] = temp;
				i++;
			}
		}
		segment_points* seg_points = (segment_points*)malloc(sizeof(segment_points));
		seg_points->points = points;
		seg_points->segmentid = row.first;
		results.push_back(seg_points);
	}

	return results;
}

measuredValue** parallel_calculate_moving_average(vector<measuredValue*> values, int moving_average_size, size_t size)
{
	measuredValue** data = (measuredValue**)malloc(sizeof(measuredValue*) * size);

	tbb::parallel_for(size_t(0), size, [&](size_t i)
	{
		float sum = 0;
		measuredValue* value = (measuredValue*)malloc(sizeof(measuredValue));
		if (value == NULL) {
			return;
		}
		if (i < moving_average_size || i + moving_average_size >= size) {
			value->ist = NULL;
		}
		else {
			for (int y = -moving_average_size; y <= moving_average_size; y++) {
				sum += values.at(i + y)->ist;
			}
			value->ist = sum / MOVING_AVERAGE;
		}

		measuredValue* segment_value = values.at(i);
		value->day = segment_value->day;
		value->second = segment_value->second;
		value->segmentid = segment_value->segmentid;
		sum = 0;
		data[i] = value;
	});

	return data;
}

map<unsigned int, vector<measuredValue*>> calculate_moving_average_tbb(map<unsigned int, vector<measuredValue*>> values_map) {
	map<unsigned int, vector<measuredValue*>> results;
	float sum = 0;

	int size = (int)MOVING_AVERAGE / 2;

	for (auto &row : values_map) {
		measuredValue** data = parallel_calculate_moving_average(row.second, size, row.second.size());
		vector<measuredValue*> temp(data, data + row.second.size());
		results[row.first] = temp;
		free(data);
	}

	return results;
}

map<unsigned int, vector<measuredValue*>> calculate_moving_average(map<unsigned int, vector<measuredValue*>> values_map) {
	map<unsigned int, vector<measuredValue*>> results;
	float sum = 0;

	int size = (int)MOVING_AVERAGE / 2;

	for (auto &row : values_map) {

		vector<measuredValue*> temp;
		results[row.first] = temp;

		for (size_t i = 0; i < row.second.size(); i++) {
			measuredValue* value = (measuredValue*)malloc(sizeof(measuredValue));
			if (value == NULL) {
				map<unsigned int, vector<measuredValue*>> free_results;
				return free_results;
			}
			if (i < size || i + size >= row.second.size()) {
				value->ist = NULL;
			}
			else {
				for (int y = -size; y <= size; y++) {
					sum += row.second.at(i + y)->ist;
				}
				value->ist = sum / MOVING_AVERAGE;
			}

			measuredValue* segment_value = row.second.at(i);
			value->day = segment_value->day;
			value->second = segment_value->second;
			value->segmentid = segment_value->segmentid;
			results.find(row.first)->second.push_back(value);
			sum = 0;
		}
	}

	return results;
}

float get_max_value_ist(vector<measuredValue*> values) {
	float max_value = 0;
	for (auto &row : values) {
		if (max_value < row->ist) {
			max_value = row->ist;
		}
	}

	return max_value;
}