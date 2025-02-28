#pragma once
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <io.h>
#include "./libs/sqlite3.h"
#include "structs.h"

using namespace std;

/*
	Trida slouzici pro praci s databazi
*/
class Database
{
public:
	Database(const char* file_name);
	~Database();

	bool open(const char* file_name);
	vector<vector<string>> query(char* query);
	vector<measured_value*> get_measured_value();
	vector<int> get_all_segments_id();
	void close();

private:
	unsigned int Database::get_seconds_of_day(int seconds);
	sqlite3 *database;
};

#endif
