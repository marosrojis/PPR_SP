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