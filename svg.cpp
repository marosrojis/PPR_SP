#include "svg.h"

SVG::SVG()
{
}

SVG::~SVG()
{
}

void SVG::print_graph(vector<point*> *values, vector<point*> *values_average, vector<segment_peaks*> peaks, size_t peaks_start_index, size_t peaks_end_index, size_t segmentid) {
	FILE * pFile;
	std::stringstream ss;

	point* x_max = (point*)malloc(sizeof(point));
	point* y_max = (point*)malloc(sizeof(point));
	point* x_min = (point*)malloc(sizeof(point));
	point* y_min = (point*)malloc(sizeof(point));
	find_max_min_x_y_points(*values, x_max, y_max, x_min, y_min);

	ss << "graph/test" << segmentid << ".svg";
	errno_t err = fopen_s(&pFile, ss.str().c_str(), "w");
	tinyxml2::XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");
	printer.PushAttribute("style", "padding: 25px 25px 15px 50px");
	printer.PushAttribute("width", x_max->x);
	printer.PushAttribute("height", y_max->y + 25);

	print_title(&printer, segmentid, x_max);
	print_axis(&printer, *values, x_max, y_max, y_min);
	print_polynate(&printer, *values, "rgb(0,0,0)");
	print_polynate(&printer, *values_average, "rgb(0,0,255)");
	
	for (size_t i = peaks_start_index; i < peaks_end_index; i++) {
		print_peaks(&printer, *(peaks.at(i)->peaks), y_max);
	}
	
	printer.CloseElement();
	std::cout << printer.CStr();

	free(x_max);
	free(y_max);
	free(x_min);
	free(y_min);
}

void SVG::print_graph_split_segment(vector<point*> *points, vector<segment_points*> points_by_day, size_t* point_position, vector<segment_peaks*> peaks, size_t* peak_position, size_t segmentid) {
	FILE * pFile;
	std::stringstream ss;

	point* x_max = (point*)malloc(sizeof(point));
	point* y_max = (point*)malloc(sizeof(point));
	point* x_min = (point*)malloc(sizeof(point));
	point* y_min = (point*)malloc(sizeof(point));
	find_max_min_x_y_points(*points, x_max, y_max, x_min, y_min);

	size_t days_in_segment = 0, y = 0;
	while (segmentid == points_by_day.at(*point_position + y)->segmentid) {
		days_in_segment++;
		y++;
	}
	for (size_t i = 0; i < days_in_segment; i++) {
		(*point_position)++;
	}
	printf("");
	
	/*ss << "graph/test" << segmentid << ".svg";
	errno_t err = fopen_s(&pFile, ss.str().c_str(), "w");
	tinyxml2::XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");
	printer.PushAttribute("style", "padding: 15px 0 15px 50px");
	printer.PushAttribute("width", x_max->x);
	printer.PushAttribute("height", (y_max->y + 25) * days_in_segment);

	print_axis(&printer, *points, x_max, y_max);
	print_polynate(&printer, *values, "rgb(0,0,0)");
	print_peaks(&printer, *points);

	printer.CloseElement();
	std::cout << printer.CStr();*/

	free(x_max);
	free(y_max);
	free(x_min);
	free(y_min);
}

void SVG::print_polynate(tinyxml2::XMLPrinter* printer, vector<point*> values, std::string color) {
	std::ostringstream retStream;

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

void SVG::print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> peaks, point* y_max) {
	for (auto &value : peaks) {
		(*printer).OpenElement("rect");
		(*printer).PushAttribute("x", value->x1);
		(*printer).PushAttribute("y", 0);
		(*printer).PushAttribute("width", value->x2 - value->x1);
		(*printer).PushAttribute("height", y_max->y + 25);
		(*printer).PushAttribute("style", "fill:green;opacity:0.25");
		(*printer).CloseElement();
	}
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

void SVG::print_axis(tinyxml2::XMLPrinter* printer, vector<point*> values, point* x_max, point* y_max, point* y_min) {
	(*printer).OpenElement("line");
	(*printer).PushAttribute("x1", 0);
	(*printer).PushAttribute("y1", 0);
	(*printer).PushAttribute("x2", 0);
	(*printer).PushAttribute("y2", y_max->y);
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 2; opacity: 0.5");
	(*printer).CloseElement();

	(*printer).OpenElement("text");
	(*printer).PushAttribute("x", 10);
	(*printer).PushAttribute("y", 0);
	(*printer).PushAttribute("style", "writing-mode: vertical-rl");
	(*printer).PushAttribute("fill", "black");
	(*printer).PushAttribute("font-size", "15");
	(*printer).PushText("ist[mmol / l]");
	(*printer).CloseElement();

	(*printer).OpenElement("line");
	(*printer).PushAttribute("x1", 0);
	(*printer).PushAttribute("y1", y_max->y);
	(*printer).PushAttribute("x2", x_max->x);
	(*printer).PushAttribute("y2", y_max->y);
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 2; opacity: 0.5");
	(*printer).CloseElement();

	(*printer).OpenElement("text");
	(*printer).PushAttribute("x", x_max->x - 95);
	(*printer).PushAttribute("y", y_max->y - 7);
	(*printer).PushAttribute("fill", "black");
	(*printer).PushAttribute("font-size", "15");
	(*printer).PushText("time [hh:mm]");
	(*printer).CloseElement();

	size_t i = 0;
	for (auto &value : values) {
		if (i % 20 == 0 && x_max->x - value->x > 30) {
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
	const int COUNT = 8;
	char number[20];
	for (i = 1; i <= COUNT; i++) {
		float value = (y_min->ist - y_max->ist) / COUNT * (COUNT - i + 1);
		sprintf(number, "%.2f", value);
		(*printer).OpenElement("text");
		(*printer).PushAttribute("x", -40);
		(*printer).PushAttribute("y", y_max->y / COUNT * i);
		(*printer).PushAttribute("fill", "black");
		(*printer).PushAttribute("font-size", "15");
		(*printer).PushText(number);
		//(*printer).PushText(y_max->ist * (i + 1));
		(*printer).CloseElement();
	}
}

void SVG::print_title(tinyxml2::XMLPrinter* printer, size_t segmentid, point* x_max) {
	std::ostringstream retStream;
	retStream << "Segment " << segmentid;

	(*printer).OpenElement("text");
	(*printer).PushAttribute("x", (x_max->x / 2) - 50);
	(*printer).PushAttribute("y", -5);
	(*printer).PushAttribute("fill", "black");
	(*printer).PushAttribute("font-size", "25");
	(*printer).PushText(retStream.str().c_str());
	(*printer).CloseElement();
}