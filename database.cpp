#include "database.h"

/* 
	Konstruktor tridy pro praci s databazi
	file_name - nazev SQLite souboru	
*/
Database::Database(const char* file_name)
{
	database = NULL;
	if (!open(file_name)) {
		cout << "Database file " << file_name << " not exist!!\n" << endl;
		exit(1);
	}
}

Database::~Database()
{
}

/* 
	Metoda pro otevreni SQLite databaze
	file_name - nazev SQLite souboru	
*/
bool Database::open(const char* file_name)
{
	if ((_access(file_name, 0)) != -1)
	{
		if (sqlite3_open(file_name, &database) == SQLITE_OK) {
			return true;
		}

		return false;
	}
	return false;
}

/*
	Provedeni dotazu pro ziskani vsech hodnot z tabulky measuredValue
*/
vector<measured_value*> Database::get_measured_value() {
	vector<vector<string>> results = query("SELECT id, ist, segmentid, strftime('%s', measuredat) FROM measuredValue WHERE ist IS NOT NULL;");
	vector<measured_value*> values;
	for (auto &row : results) {
		measured_value* value = (measured_value*) malloc(sizeof(measured_value));
		if (value == nullptr) {
			for (size_t i = 0; i < values.size(); i++) {
				free(values.at(i));
			}
			cout << "Malloc memory error" << endl;
			vector<measured_value*> free_result;
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

/*
	Ziskani vsech existujicich ID segmentu
*/
vector<int> Database::get_all_segments_id() {
	vector<vector<string>> results = query("SELECT id FROM timesegment ORDER BY id;");
	vector<int> segmentsId;

	for (size_t i = 0; i < results.size(); i++) {
		segmentsId.push_back(stoi(results.at(i).at(0)));
	}

	return segmentsId;
}

/*
	Metoda pro provedeni SELECT dotazu pro ziskani dat z DB
	query - SQL dotaz
*/
vector<vector<string>> Database::query(char* query)
{
	sqlite3_stmt *statement;
	vector<vector<string>> results;

	if (sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		int result = 0;
		while (true) {
			result = sqlite3_step(statement);

			if (result == SQLITE_ROW) {
				vector<string> values;
				for (int col = 0; col < cols; col++) {
					std::string  val;
					char * ptr = (char*)sqlite3_column_text(statement, col);

					if (ptr) {
						val = ptr;
					}
					else val = "";

					values.push_back(val);
				}
				results.push_back(values);
			}
			else {
				break;
			}

		}

		sqlite3_finalize(statement);
	}

	string error = sqlite3_errmsg(database);
	if (error != "not an error") cout << query << " " << error << endl;

	return results;
}

/*
	Uzavreni spojeni s databazi
*/
void Database::close()
{
	sqlite3_close(database);
}

/*
	Metoda pro ziskani poctu sekund od zacatku dne
*/
unsigned int Database::get_seconds_of_day(int seconds) {
	const unsigned int cseconds_in_day = 86400;

	unsigned int seconds_of_day = seconds - (((int)(seconds / cseconds_in_day)) * cseconds_in_day);
	return seconds_of_day;

}