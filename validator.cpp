#include "validator.h"

using namespace std;

/*
	Vytisknuti napovedy pro pouziti aplikace
*/
void print_help() {
	ostringstream ret_stream;

	ret_stream << "Usage: PPR.exe [-h] [-serial | -tbb | -gpu] [-stats] [-d]\n\n";
	ret_stream << "Options:\n";
	ret_stream << "\t-d\t\t Split segment on days in graph.\n";
	ret_stream << "\t-h\t\t Show help.\n";
	ret_stream << "\t-gpu\t\t Start parallel version using GPU.\n";
	ret_stream << "\t-serial\t\t Start serial version.\n";
	ret_stream << "\t-tbb\t\t Start parallel version using Threading Building Blocks.\n\n";

	cout << ret_stream.str();
}

/*
	Validace vstupnich parametru od uzivatele. Funkce vrati strukturu config obsahujici nastaveni pro spusteni aplikace.

	args - vstupni parametry uzivatele
*/
config* validate_input(vector<string> args) {
	config* cfg = (config*)malloc(sizeof(config));

	for (size_t i = 0; i < args.size(); i++) {
		string input = args.at(i);
		if (!(input == "-h" || input == "-serial" || input == "-tbb" || input == "-gpu" || input == "-stats" || input == "-d" || input == "-f" || input == "-start")) {
			if (i != 0 && (args.at(i - 1) == "-tbb" || args.at(i - 1) == "-f" || args.at(i - 1) == "-start")) { // validate number of threads or DB file name
				continue;
			}
			cout << "Invalid argument \"" << input << ".\"\n" << endl;
			print_help();
			cfg->valid_input = false;
			return cfg;
		}
	}

	if (args.size() == 0 || find(args.begin(), args.end(), "-h") != args.end()) {
		print_help();
		cfg->valid_input = false;
		return cfg;
	}

	if (!(find(args.begin(), args.end(), "-serial") != args.end() || find(args.begin(), args.end(), "-tbb") != args.end() ||
		find(args.begin(), args.end(), "-gpu") != args.end())) {
		cout << "Select version of program (serial, tbb or gpu).\n\n" << endl;
		print_help();
		cfg->valid_input = false;
		return cfg;
	}

	if (find(args.begin(), args.end(), "-f") != args.end()) {
		vector<string>::iterator it = find(args.begin(), args.end(), "-f");
		size_t pos = it - args.begin();
		if (pos + 1 == args.size() || args.at(pos + 1).at(0) == '-') {
			cout << "Need set name of database file.\n\n" << endl;
			print_help();
			cfg->valid_input = false;
			return cfg;
		}

		cfg->db_file_name = args.at(pos + 1).c_str();
	}
	else {
		cfg->db_file_name = "data/direcnet.sqlite";
	}

	if ((find(args.begin(), args.end(), "-serial") != args.end() && find(args.begin(), args.end(), "-tbb") != args.end()) ||
		(find(args.begin(), args.end(), "-serial") != args.end() && find(args.begin(), args.end(), "-gpu") != args.end()) ||
		find(args.begin(), args.end(), "-gpu") != args.end() && find(args.begin(), args.end(), "-tbb") != args.end()) {
		cout << "Use only one version of program!\n\n" << endl;
		print_help();
		cfg->valid_input = false;
		return cfg;
	}

	if (find(args.begin(), args.end(), "-serial") != args.end()) {
		cfg->run_serial = true;
		cfg->run_tbb = false;
		cfg->run_gpu = false;
	}
	else if (find(args.begin(), args.end(), "-tbb") != args.end()) {
		cfg->run_serial = false;
		cfg->run_tbb = true;
		cfg->run_gpu = false;
	}
	else {
		cfg->run_serial = false;
		cfg->run_tbb = false;
		cfg->run_gpu = true;
	}

	if (find(args.begin(), args.end(), "-start") != args.end()) {
		vector<string>::iterator it = find(args.begin(), args.end(), "-start");
		size_t pos = it - args.begin();
		if (pos + 1 == args.size() || args.at(pos + 1).at(0) == '-') {
			cout << "Need set number of start calculations.\n\n" << endl;
			print_help();
			cfg->valid_input = false;
			return cfg;
		}

		string start_number = args.at(pos + 1);
		char * p;
		long ret = strtol(start_number.c_str(), &p, 10);

		if (*p == 0) {
			cfg->number_of_start = atoi(start_number.c_str());
		}
		else {
			cout << "Need set number of start calculations.\n\n" << endl;
			print_help();
			cfg->valid_input = false;
			return cfg;
		}
	}
	else {
		cfg->number_of_start = 1;
	}

	cfg->print_stats = find(args.begin(), args.end(), "-stats") != args.end();
	cfg->split_segments = find(args.begin(), args.end(), "-d") != args.end();

	cfg->valid_input = true;
	return cfg;
}