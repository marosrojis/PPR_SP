#include "graph.h"

using namespace std;

bool compareBySum(const peak* a, const peak* b) {
	return a->sum > b->sum;
}

segment_peaks* create_segment_peaks(vector<peak*> *peaks, size_t segmentid) {
	vector<peak*> peaks_in_day = *peaks;
	sort(peaks_in_day.begin(), peaks_in_day.end(), compareBySum);

	size_t peak_size = peaks_in_day.size() >= SHOW_PEAKS ? SHOW_PEAKS : peaks_in_day.size();
	vector<peak*> *sort_peaks = new vector<peak*>(peak_size);
	for (size_t i = 0; i < peaks_in_day.size(); i++) {
		peak* tmp = peaks_in_day.at(i);
		if (i < peak_size) {
			(*sort_peaks)[i] = tmp;
		}
		else {
			free(tmp);
		}
	}

	segment_peaks* seg_peaks = (segment_peaks*)malloc(sizeof(segment_peaks));
	if (seg_peaks == nullptr) {
		delete(sort_peaks);
		printf("Malloc memory error\n");
		return nullptr;
	}
	seg_peaks->peaks = sort_peaks;
	seg_peaks->segmentid = segmentid;

	return seg_peaks;
}

segment_peaks*** parallel_get_peaks(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** peaks_segment, size_t size) {
	segment_peaks*** data = (segment_peaks***)malloc(sizeof(segment_peaks**) * size);
	if (data == nullptr) {
		printf("Malloc memory error\n");
		return nullptr;
	}

	tbb::task_scheduler_init init(4);
	tbb::parallel_for(size_t(0), size, [&](size_t a) {
		vector<point*> segment = *(points.at(a)->points);

		bool is_peak = false;
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
					if (point_base_line->x - temp_peak->x >= MIN_MINUTE_FOR_ACTION && grow < 3) {
						peak* temp = (peak*)malloc(sizeof(peak));
						if (temp == nullptr) {
							printf("Malloc memory error\n");
							return;
						}

						temp->x1 = temp_peak;
						temp->x2 = point_base_line;
						temp->sum = sum;
						peaks.push_back(temp);

					}
					is_peak = false;
				}
			}
			i++;
		}

		size_t seg_day = a;
		while (points.at(a)->segmentid != points_by_day.at(seg_day)->segmentid) {
			seg_day++;
		}
		vector<segment_peaks*> peaks_in_segment;
		vector<peak*> peaks_in_day;
		for (auto &peak_seg : peaks) {
			float start_day = points_by_day.at(seg_day)->points->at(0)->x;
			float end_day = points_by_day.at(seg_day)->points->at(points_by_day.at(seg_day)->points->size() - 1)->x;
			if (peak_seg->x1->x >= start_day && peak_seg->x1->x <= end_day) {
				peaks_in_day.push_back(peak_seg);
			}
			else {
				segment_peaks* seg_peaks = create_segment_peaks(&peaks_in_day, points.at(a)->segmentid);
				peaks_in_day.clear();
				peaks_in_day.push_back(peak_seg);
				peaks_in_segment.push_back(seg_peaks);
				seg_day++;
			}
		}

		if (peaks_in_day.size() != 0) {
			segment_peaks* seg_peaks = create_segment_peaks(&peaks_in_day, points.at(a)->segmentid);
			peaks_in_day.clear();
			peaks_in_segment.push_back(seg_peaks);
			seg_day++;
		}
		
		segment_peaks** result = (segment_peaks**)malloc(sizeof(segment_peaks*) * peaks_in_segment.size());
		if (result == nullptr) {
			printf("Malloc memory error\n");
			return;
		}

		copy(peaks_in_segment.begin(), peaks_in_segment.end(), stdext::checked_array_iterator<segment_peaks**>(result, peaks_in_segment.size()));
		(*peaks_segment)[a] = peaks_in_segment.size();

		data[a] = result;
	});

	return data;
}

vector<segment_peaks*> get_peaks_tbb(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** peak_segment_position) {
	vector<segment_peaks*> results;
	size_t* peaks_segment = (size_t*)malloc(sizeof(size_t) * points.size());
	if (peaks_segment == nullptr) {
		printf("Malloc memory error\n");
		return results;
	}
	segment_peaks*** data = parallel_get_peaks(points, points_average, points_by_day, &peaks_segment, points.size());

	size_t pos = 0;
	for (size_t i = 0; i < points.size(); i++) {
		(*peak_segment_position)[i] = pos;
		for (size_t y = 0; y < peaks_segment[i]; y++) {
			results.push_back(data[i][y]);
			pos++;
		}
		free(data[i]);
	}

	free(data);
	free(peaks_segment);

	return results;
}

vector<segment_peaks*> get_peaks(vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** segments_position) {
	vector<segment_peaks*> results;
	size_t seg_day = 0;

	for (size_t a = 0; a < points.size(); a++) {
		vector<point*> segment = *(points.at(a)->points);

		bool is_peak = false;
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
					if (point_base_line->x - temp_peak->x >= MIN_MINUTE_FOR_ACTION && grow < 3) {
						peak* temp = (peak*)malloc(sizeof(peak));
						if (temp == nullptr) {
							printf("Malloc memory error\n");
							return results;
						}

						temp->x1 = temp_peak;
						temp->x2 = point_base_line;
						temp->sum = sum;
						peaks.push_back(temp);

					}
					is_peak = false;
				}
			}
			i++;
		}
		
		(*segments_position)[a] = results.size();
		while (points.at(a)->segmentid != points_by_day.at(seg_day)->segmentid) {
			seg_day++;
		}
		vector<peak*> peaks_in_day;
		for (auto &peak_seg : peaks) {
			float start_day = points_by_day.at(seg_day)->points->at(0)->x;
			float end_day = points_by_day.at(seg_day)->points->at(points_by_day.at(seg_day)->points->size() - 1)->x;
			if (peak_seg->x1->x >= start_day && peak_seg->x1->x <= end_day) {
				peaks_in_day.push_back(peak_seg);
			}
			else {
				segment_peaks* seg_peaks = create_segment_peaks(&peaks_in_day, points.at(a)->segmentid);
				peaks_in_day.clear();
				peaks_in_day.push_back(peak_seg);
				results.push_back(seg_peaks);
				seg_day++;
			}
		}

		if (peaks_in_day.size() != 0) {
			segment_peaks* seg_peaks = create_segment_peaks(&peaks_in_day, points.at(a)->segmentid);
			results.push_back(seg_peaks);
		}
	}
	return results;
}

map<size_t, float> get_max_values(map<size_t, vector<measured_value*>> values) {
	map<size_t, float> results;
	for (auto &value : values) {
		float max_value = get_max_value_ist(value.second);
		results[value.first] = max_value;
	}

	return results;
}

vector<segment_points*> split_segments_by_day(vector<segment_points*> segments) {
	vector<segment_points*> results;

	for (auto &row : segments) {
		size_t i = 0;
		size_t vectorSize = row->points->size();
		vector<point*> *points = new vector<point*>(vectorSize);

		int64_t lastDay = row->points->at(0)->second;
		for (size_t y = 0; y < row->points->size(); y++) {
			point* value = row->points->at(y);		
			if (lastDay > value->second) {
				points->resize(i);
				segment_points* seg_points = (segment_points*)malloc(sizeof(segment_points));
				if (seg_points == nullptr) {
					printf("Malloc memory error\n");
					delete(points);
					return results;
				}

				seg_points->points = points;
				seg_points->segmentid = row->segmentid;
				results.push_back(seg_points);

				vectorSize -= i;
				points = new vector<point*>(vectorSize);
				i = 0;
			}
			(*points)[i] = value;
			i++;
			lastDay = value->second;
		}
		segment_points* seg_points = (segment_points*)malloc(sizeof(segment_points));
		if (seg_points == nullptr) {
			printf("Malloc memory error\n");
			delete(points);
			return results;
		}
		seg_points->points = points;
		seg_points->segmentid = row->segmentid;
		results.push_back(seg_points);
	}

	return results;
}

vector<segment_points*> get_points_from_values(map<size_t, vector<measured_value*>> values, map<size_t, float> max_values, bool isAverage) {
	vector<segment_points*> results;
	for (auto &row : values) {
		size_t i = 0;
		size_t vectorSize = (isAverage ? row.second.size() - MOVING_AVERAGE + 1: row.second.size());

		vector<point*> *points = new vector<point*>(vectorSize);

		unsigned int lastSecond = row.second.at(0)->second;
		for (auto &value : row.second) {
			if (value->ist != NULL) {
				point* temp = (point *)malloc(sizeof(point));
				if (temp == nullptr) {
					printf("Malloc memory error\n");
					delete(points);
					return results;
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
		if (seg_points == nullptr) {
			printf("Malloc memory error\n");
			delete(points);
			return results;
		}
		seg_points->points = points;
		seg_points->segmentid = row.first;
		results.push_back(seg_points);
	}

	return results;
}

measured_value** parallel_calculate_moving_average(vector<measured_value*> values, int moving_average_size, size_t size)
{
	measured_value** data = (measured_value**)malloc(sizeof(measured_value*) * size);
	if (data == nullptr) {
		printf("Malloc memory error\n");
		return nullptr;
	}

	tbb::parallel_for(size_t(0), size, [&](size_t i)
	{
		float sum = 0;
		measured_value* value = (measured_value*)malloc(sizeof(measured_value));
		if (value == nullptr) {
			printf("Malloc memory error\n");
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

		measured_value* segment_value = values.at(i);
		value->second = segment_value->second;
		value->segmentid = segment_value->segmentid;
		sum = 0;
		data[i] = value;
	});

	return data;
}

map<size_t, vector<measured_value*>> calculate_moving_average_tbb(map<size_t, vector<measured_value*>> values_map) {
	map<size_t, vector<measured_value*>> results;
	float sum = 0;

	int size = (int)MOVING_AVERAGE / 2;

	for (auto &row : values_map) {
		measured_value** data = parallel_calculate_moving_average(row.second, size, row.second.size());
		vector<measured_value*> temp(data, data + row.second.size());
		results[row.first] = temp;
		free(data);
	}

	return results;
}

map<size_t, vector<measured_value*>> calculate_moving_average(map<size_t, vector<measured_value*>> values_map) {
	map<size_t, vector<measured_value*>> results;
	float sum = 0;

	int size = (int)MOVING_AVERAGE / 2;

	for (auto &row : values_map) {

		vector<measured_value*> temp;
		results[row.first] = temp;

		for (size_t i = 0; i < row.second.size(); i++) {
			measured_value* value = (measured_value*)malloc(sizeof(measured_value));
			if (value == nullptr) {
				printf("Malloc memory error\n");
				return results;
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

			measured_value* segment_value = row.second.at(i);
			value->second = segment_value->second;
			value->segmentid = segment_value->segmentid;
			results.find(row.first)->second.push_back(value);
			sum = 0;
		}
	}

	return results;
}

float get_max_value_ist(vector<measured_value*> values) {
	float max_value = 0;
	for (auto &row : values) {
		if (max_value < row->ist) {
			max_value = row->ist;
		}
	}

	return max_value;
}