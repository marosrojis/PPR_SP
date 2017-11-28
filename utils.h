#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <sstream>
#include "structs.h"

const int SECOND_TO_MINUTE = 60;

point* get_max_x_point(std::vector<point*> &values);
point* get_max_y_point(std::vector<point*> &values);
void find_max_min_x_y_points(std::vector<point*> &values, point* x_max_point, point* y_max_point, point* x_min_point, point* y_min_point);
std::string get_time(unsigned int seconds);

#endif