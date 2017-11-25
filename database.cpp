#include "database.h"
#include <iostream>

Database::Database()
{
	database = NULL;
	open("data/direcnet.sqlite");
}

Database::~Database()
{
}

bool Database::open(char* filename)
{
	if (sqlite3_open(filename, &database) == SQLITE_OK)
		return true;

	return false;
}

vector<measuredValue*> Database::get_measured_value() {
	vector<vector<string>> results = query("SELECT id, ist, segmentid, strftime('%s', measuredat) FROM measuredvalue WHERE ist IS NOT NULL;");
	vector<measuredValue*> values;
	for (auto &row : results) // access by reference to avoid copying
	{
		measuredValue* value = (measuredValue*) malloc(sizeof(measuredValue));
		if (value == nullptr) {
			for (size_t i = 0; i < values.size(); i++) {
				free(values.at(i));
			}
			cout << "Malloc memory error";
			vector<measuredValue*> free_result;
			return free_result;
		}
		value->id = stoi(row.at(0));
		value->ist = stof(row.at(1));
		value->segmentid = stoi(row.at(2));
		value->second = stoi(row.at(3));
		value->second_of_day = get_seconds_of_day(value->second);
		
		values.push_back(value);
	}

	return values;
}

vector<int> Database::get_all_segments_id() {
	vector<vector<string>> results = query("SELECT id FROM timesegment ORDER BY id;");
	vector<int> segmentsId;

	for (size_t i = 0; i < results.size(); i++) {
		segmentsId.push_back(stoi(results.at(i).at(0)));
	}

	return segmentsId;
}

vector<vector<string>> Database::query(char* query)
{
	sqlite3_stmt *statement;
	vector<vector<string>> results;

	if (sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		int result = 0;
		while (true)
		{
			result = sqlite3_step(statement);

			if (result == SQLITE_ROW)
			{
				vector<string> values;
				for (int col = 0; col < cols; col++)
				{
					std::string  val;
					char * ptr = (char*)sqlite3_column_text(statement, col);

					if (ptr)
					{
						val = ptr;
					}
					else val = ""; // this can be commented out since std::string  val;
								   // initialize variable 'val' to empty string anyway

					values.push_back(val);  // now we will never push NULL
				}
				results.push_back(values);
			}
			else
			{
				break;
			}

		}

		sqlite3_finalize(statement);
	}

	string error = sqlite3_errmsg(database);
	if (error != "not an error") cout << query << " " << error << endl;

	return results;
}

void Database::close()
{
	sqlite3_close(database);
}

unsigned int Database::get_seconds_of_day(int seconds) {
	const unsigned int cseconds_in_day = 86400;

	unsigned int seconds_of_day = seconds - (((int)(seconds / cseconds_in_day)) * cseconds_in_day);
	return seconds_of_day;

}