#include "graph.h"

using namespace std;

map<unsigned int, vector<peak*>> get_peaks(map<unsigned int, vector<point*>> points, map<unsigned int, vector<point*>> points_average) {
	map<unsigned int, vector<peak*>> results;

	for (auto &segment : points) {
		
		bool is_peak = false;
		float x_peak = 0, y_peak = 0;
		size_t i = 0;
		vector<peak*> peaks;
		for (auto &point_base_line : segment.second) {
			if (i >= points_average.find(segment.first)->second.size()) { // i je >= nez celkova velikost average vectoru, tak konec cyklu (uz neni kam sahat)
				break;
			}

			point* average_line = points_average.find(segment.first)->second.at(i);
			if (point_base_line->x != average_line->x) { // tato podminka je kvuli zacatku points vektoru, protoze obsahuje body, ktery points_average neobsahuje
				continue;
			}
			
			if (point_base_line->y <= average_line->y) {
				if (!is_peak) {
					x_peak = point_base_line->x;
					y_peak = point_base_line->y;
					is_peak = true;
				}
			}
			else {
				if (is_peak) {
					if (point_base_line->x - x_peak >= MIN_SECOND_FOR_ACTION) {
						peak* temp = (peak*)malloc(sizeof(peak));
						temp->x1 = x_peak;
						temp->x2 = point_base_line->x;
						temp->y1 = y_peak;
						temp->y2 = point_base_line->y;
						peaks.push_back(temp);
						
					}
					is_peak = false;
				}
			}
			i++;
		}
		results[segment.first] = peaks;
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

map<unsigned int, vector<point*>> get_points_from_values(map<unsigned int, vector<measuredValue*>> values, map<unsigned int, float> max_values) {
	map<unsigned int, vector<point*>> results;

	for (auto &row : values) {
		vector<point*> points;

		unsigned int lastSecond = row.second.at(0)->second;
		for (auto &value : row.second) {
			if (value->ist != NULL) {
				point* temp = (point *)malloc(sizeof(point));
				temp->x = static_cast<float>((value->second - lastSecond) / MINUTE);
				temp->y = (max_values.find(row.first)->second - value->ist) * Y_SCALE;
				temp->second = value->second_of_day;
				temp->ist = (int)value->ist;
				points.push_back(temp);
			}
		}
		results[row.first] = points;
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