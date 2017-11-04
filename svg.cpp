#include "svg.h"

SVG::SVG()
{
}

SVG::~SVG()
{
}

void SVG::print_graph(vector<point*> values, vector<point*> values_average, vector<peak*> peaks, int segmentid) {
	print_graph(values, values_average, peaks, segmentid, "rgb(0,0,0)", "rgb(0,0,255)");
}

void SVG::print_graph(vector<point*> values, vector<point*> values_average, vector<peak*> peaks, int segmentid, std::string color1, std::string color2) {
	FILE * pFile;
	std::stringstream ss;
	ss << "graph/test" << segmentid << ".xml";
	errno_t err = fopen_s(&pFile, ss.str().c_str(), "w");
	tinyxml2::XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");
	printer.PushAttribute("style", "padding: 15px 10px");
	printer.PushAttribute("width", get_max_x_point(values));
	printer.PushAttribute("height", get_max_y_point(values));

	print_border(&printer);
	print_polynate(&printer, values, color1);
	print_polynate(&printer, values_average, color2);
	float average = print_average_line(&printer, values);
	print_peaks(&printer, peaks, average);
	
	printer.CloseElement();
	std::cout << printer.CStr();
}

void SVG::print_polynate(tinyxml2::XMLPrinter* printer, vector<point*> values, std::string color) {
	std::ostringstream retStream;
	std::string points;

	for (auto &row : values) {
		retStream << row->x << "," << row->y << " ";
	}

	(*printer).OpenElement("polyline");
	(*printer).PushAttribute("points", retStream.str().c_str());
	(*printer).PushAttribute("stroke", color.c_str());
	(*printer).PushAttribute("stroke-width", "1.5");
	(*printer).PushAttribute("fill", "transparent");
	(*printer).CloseElement();
}

void SVG::print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> peaks, float average) {
	for (auto &value : peaks) {
		if (value->y1 < average && value->y2 < average) {
			(*printer).OpenElement("rect");
			(*printer).PushAttribute("x", value->x1);
			(*printer).PushAttribute("y", 0);
			(*printer).PushAttribute("width", value->x2 - value->x1);
			(*printer).PushAttribute("height", "100%");
			(*printer).PushAttribute("style", "fill:green;opacity:0.25");
			(*printer).CloseElement();
		}
	}
}

float SVG::print_average_line(tinyxml2::XMLPrinter* printer, vector<point*> points) {
	float sum = 0;
	for (auto &value: points) {
		sum += value->y;
	}
	float average = sum / points.size();
	(*printer).OpenElement("line");
	(*printer).PushAttribute("x1", 0);
	(*printer).PushAttribute("y1", average);
	(*printer).PushAttribute("x2", "100%");
	(*printer).PushAttribute("y2", average);
	(*printer).PushAttribute("style", "stroke:rgb(200, 200,200); stroke-width:1.5; opacity: 0.25");
	(*printer).CloseElement();

	return average;
}


void SVG::print_border(tinyxml2::XMLPrinter* printer) {
	(*printer).OpenElement("rect");
	(*printer).PushAttribute("x", 0);
	(*printer).PushAttribute("y", 0);
	(*printer).PushAttribute("width", "100%");
	(*printer).PushAttribute("height", "100%");
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 2; opacity: 0.5");
	(*printer).CloseElement();
}

float SVG::get_max_x_point(vector<point*> values) {
	float max = 0;
	for (auto & value : values) {
		if (max < value->x) {
			max = value->x;
		}
	}
	return max;
}

float SVG::get_max_y_point(vector<point*> values) {
	float max = 0;
	for (auto & value : values) {
		if (max < value->y) {
			max = value->y;
		}
	}
	return max;
}