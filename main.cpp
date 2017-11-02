#include <ios>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <map>
#include "database.h"
#include "tinyxml2.h"

using namespace std;
using namespace tinyxml2;

Database *db;

int test2() {
	
	vector<vector<string>> result = db->query("SELECT * FROM subject;");
	for (vector<vector<string>>::iterator it = result.begin(); it < result.end(); ++it)
	{
		vector<string> row = *it;
		cout << "Values: (A=" << row.at(0) << ", B=" << row.at(2) << ")" << endl;
	}
	db->close();

	return 1;
}

void printPolynate(XMLPrinter* printer, vector<measuredValue*> values) {
	std::ostringstream retStream;
	std::string points;
	unsigned int lastSecond = values.at(0)->second;
	float max_value = 0;

	for (auto &row : values) {
		if (max_value < row->ist) {
			max_value = row->ist;
		}
	}

	for (auto &row : values) {
		retStream << (row->second - lastSecond) / 60 << "," << (max_value - row->ist) * 20 << " ";
		
		//cout << "ID=" << row->id << ", ist=" << row->ist << ", segmentid=" << row->segmentid << ", second=" << row->second << ", day=" << row->day << endl;
	}

	(*printer).OpenElement("polyline");
	(*printer).PushAttribute("points", retStream.str().c_str());
	(*printer).PushAttribute("stroke", "rgb(0,0,0)");
	(*printer).PushAttribute("stroke-width", "0.5");
	(*printer).PushAttribute("fill", "transparent");
}

void printGraph(vector<measuredValue*> values) {
	FILE * pFile;
	std::stringstream ss;
	ss << "test" << values.at(0)->segmentid << ".xml";
	errno_t err = fopen_s(&pFile, ss.str().c_str(), "w");
	XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");

	printPolynate(&printer, values);

	printer.CloseElement();
	printer.CloseElement();
	cout << printer.CStr();
}

void printAllSegments(map<unsigned int, vector<measuredValue*>> values) {
	for (auto &row : values) {
		printGraph(row.second);
	}

}

map<unsigned int, vector<measuredValue*>> transform_measured_value(vector<measuredValue*> values) {
	map<unsigned int, vector<measuredValue*>> values_map;
	for (auto &row : values) {
		if (row->ist == NULL) {
			continue;
		}
		map<unsigned int, vector<measuredValue*>>::iterator p = values_map.find(row->segmentid);
		if (p != values_map.end()) {
			p->second.push_back(row);
		}
		else {
			vector<measuredValue*> temp;
			temp.push_back(row);
			values_map[row->segmentid] = temp;
		}
	}
	return values_map;
}

int main()
{
	db = new Database();
	vector<int> segmentsId = db->get_all_segments_id();

	vector<measuredValue*> values = db->get_measured_value();
	map<unsigned int, vector<measuredValue*>> values_map = transform_measured_value(values);

	printAllSegments(values_map);

	//test2();

	// Wait For User To Close Program
	/*cout << "Please press any key to exit the program ..." << endl;
	cin.get();*/

	return 0;

}

