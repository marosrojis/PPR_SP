#include "utils.h"

using namespace std;

point* get_max_x_point(vector<point*> values) {
	float max = 0;
	point* point_max = NULL;
	for (auto & value : values) {
		if (max < value->x) {
			max = value->x;
			point_max = value;
		}
	}
	return point_max;
}

point* get_max_y_point(vector<point*> values) {
	float max = 0;
	point* point_max = NULL;
	for (auto & value : values) {
		if (max < value->y) {
			max = value->y;
			point_max = value;
		}
	}
	return point_max;
}

void find_max_min_x_y_points(vector<point*> values, point* x_max_point, point* y_max_point, point* x_min_point, point* y_min_point) {
	float x_max = 0, y_max = 0, x_min = values.at(0)->x, y_min = values.at(0)->y;
	
	if (x_min_point != nullptr) {
		memcpy(x_min_point, values.at(0), sizeof(point));
	}
	if (y_min_point != nullptr) {
		memcpy(y_min_point, values.at(0), sizeof(point));
	}
	for (auto & value : values) {
		if (x_max_point != nullptr && x_max < value->x) {
			x_max = value->x;
			memcpy(x_max_point, value, sizeof(point));
		}
		if (y_max < value->y) {
			y_max = value->y;
			memcpy(y_max_point, value, sizeof(point));
		}
		if (x_min_point != nullptr && x_min > value->x) {
			x_min = value->x;
			memcpy(x_min_point, value, sizeof(point));
		}
		if (y_min_point != nullptr && y_min > value->y) {
			y_min = value->y;
			memcpy(y_min_point, value, sizeof(point));
		}
	}
}

string get_time(unsigned int seconds) {
	int hours, minutes;

	minutes = seconds / SECOND_TO_MINUTE;
	hours = minutes / SECOND_TO_MINUTE;

	minutes = minutes % SECOND_TO_MINUTE;

	ostringstream retStream;
	if (std::to_string(hours).length() == 1) {
		retStream << "0";
	}
	retStream << hours << ":";
	if (std::to_string(minutes).length() == 1) {
		retStream << "0";
	}
	retStream << minutes;

	string time = retStream.str();
	return time;
}