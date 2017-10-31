// PPR_SP.cpp : Defines the entry point for the console application.
//

#include <ios>
#include <iostream>
//#include "./libs/sqlite3.h"
#include "database.h"

using namespace std;

int test() {
	int rc;
	char *error;

	// Open Database
	cout << "Opening MyDb.db ..." << endl;
	sqlite3 *db;
	rc = sqlite3_open("data/direcnet.sqlite", &db);
	if (rc)
	{
		cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_close(db);
		return 1;
	}
	else
	{
		cout << "Opened MyDb.db." << endl << endl;
	}

	// Display MyTable
	cout << "Retrieving values in MyTable ..." << endl;
	const char *sqlSelect = "SELECT * FROM timesegment;";
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
	}
	else
	{
		// Display Table
		for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				// Determine Cell Position
				int cellPosition = (rowCtr * columns) + colCtr;

				// Display Cell Value
				cout.width(12);
				cout.setf(ios::left);
				cout << results[cellPosition] << " ";
			}

			// End Line
			cout << endl;

			// Display Separator For Header
			if (0 == rowCtr)
			{
				for (int colCtr = 0; colCtr < columns; ++colCtr)
				{
					cout.width(12);
					cout.setf(ios::left);
					cout << "~~~~~~~~~~~~ ";
				}
				cout << endl;
			}
		}
	}


	// Close Database
	cout << "Closing MyDb.db ..." << endl;
	sqlite3_close(db);
	cout << "Closed MyDb.db" << endl << endl;

	return 1;
}

int test2() {
	Database *db;
	db = new Database("data/direcnet.sqlite");
	vector<vector<string>> result = db->query("SELECT * FROM subject;");
	for (vector<vector<string>>::iterator it = result.begin(); it < result.end(); ++it)
	{
		vector<string> row = *it;
		cout << "Values: (A=" << row.at(0) << ", B=" << row.at(2) << ")" << endl;
	}
	db->close();

	return 1;
}

int main()
{
	test2();

	// Wait For User To Close Program
	cout << "Please press any key to exit the program ..." << endl;
	cin.get();

	return 0;

}

