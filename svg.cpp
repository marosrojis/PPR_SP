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

	/*point* x_max = get_max_x_point(values);
	point* y_max = get_max_y_point(values);*/
	point* x_max = (point*)malloc(sizeof(point));
	point* y_max = (point*)malloc(sizeof(point));
	find_max_x_y_points(values, x_max, y_max);

	ss << "graph/test" << segmentid << ".svg";
	errno_t err = fopen_s(&pFile, ss.str().c_str(), "w");
	tinyxml2::XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");
	printer.PushAttribute("style", "padding: 15px 0 15px 50px");
	printer.PushAttribute("width", x_max->x);
	printer.PushAttribute("height", y_max->y + 25);

	print_axis(&printer, values, x_max, y_max);
	print_polynate(&printer, values, color1);
	print_polynate(&printer, values_average, color2);
	float average = print_average_line(&printer, values);
	print_peaks(&printer, peaks, average);
	
	printer.CloseElement();
	std::cout << printer.CStr();

	free(x_max);
	free(y_max);
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
		//if (value->y1 < average && value->y2 < average) {
			(*printer).OpenElement("rect");
			(*printer).PushAttribute("x", value->x1);
			(*printer).PushAttribute("y", 0);
			(*printer).PushAttribute("width", value->x2 - value->x1);
			(*printer).PushAttribute("height", "100%");
			(*printer).PushAttribute("style", "fill:green;opacity:0.25");
			(*printer).CloseElement();
		//}
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

void SVG::print_axis(tinyxml2::XMLPrinter* printer, vector<point*> values, point* x_max, point* y_max) {
	(*printer).OpenElement("line");
	(*printer).PushAttribute("x1", 0);
	(*printer).PushAttribute("y1", 0);
	(*printer).PushAttribute("x2", 0);
	(*printer).PushAttribute("y2", y_max->y);
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 2; opacity: 0.5");
	(*printer).CloseElement();

	(*printer).OpenElement("line");
	(*printer).PushAttribute("x1", 0);
	(*printer).PushAttribute("y1", y_max->y);
	(*printer).PushAttribute("x2", x_max->x);
	(*printer).PushAttribute("y2", y_max->y);
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 2; opacity: 0.5");
	(*printer).CloseElement();

	size_t i = 0;
	for (auto &value : values) {
		if (i % 10 == 0 && x_max->x - value->x > 30) {
			(*printer).OpenElement("text");
			(*printer).PushAttribute("x", value->x);
			(*printer).PushAttribute("y", y_max->y + 20);
			(*printer).PushAttribute("fill", "black");
			(*printer).PushAttribute("font-size", "15");
			(*printer).PushText(get_time(value->second).c_str());
			(*printer).CloseElement();
		}
		i++;
	}

	for (i = 1; i <= 10; i++) {
		(*printer).OpenElement("text");
		(*printer).PushAttribute("x", -20);
		(*printer).PushAttribute("y", y_max->y / 10 * i);
		(*printer).PushAttribute("fill", "black");
		(*printer).PushAttribute("font-size", "15");
		(*printer).PushText((int)((int)y_max->ist * (10 - i)));
		//(*printer).PushText(y_max->ist * (i + 1));
		(*printer).CloseElement();
	}
}