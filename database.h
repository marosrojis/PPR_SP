#pragma once
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <string>
#include <sstream>
#include <vector>
#include "./libs/sqlite3.h"
#include "structs.h"

using namespace std;

class Database
{
public:
	Database();
	~Database();

	bool open(char* filename);
	vector<vector<string>> query(char* query);
	vector<measuredValue*> get_measured_value();
	vector<int> get_all_segments_id();
	void close();

private:
	unsigned int Database::get_seconds_of_day(int seconds);
	sqlite3 *database;
};

#endif
