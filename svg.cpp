#include "svg.h"

SVG::SVG()
{
}

SVG::~SVG()
{
}

/*
	Funkce pro vytvoreni grafu segmentu

	values - vektor obsahujici body segmentu, ktere se maji vykreslit
	values_average - vektor obsahujici body segmentu vzniklych klouzavym prumerem, ktere se maji vykreslit
	peaks - vektor obsahujici vsechny vykyvy
	peaks_start_index - index pro prvni vykyv vykreslovaneho segmentu
	peaks_end_index - index pro posledni vykyv vykreslovaneho segmentu
	segmentid - ID segmentu
*/
void SVG::print_graph(vector<point*> *values, vector<point*> *values_average, vector<segment_peaks*> peaks, size_t peaks_start_index, size_t peaks_end_index, size_t segmentid) {
	FILE * pFile;
	std::stringstream ss;

	point* x_max = (point*)malloc(sizeof(point));
	if (x_max == nullptr) {
		printf("Malloc memory error\n");
		return;
	}
	point* y_max = (point*)malloc(sizeof(point));
	if (y_max == nullptr) {
		printf("Malloc memory error\n");
		free(x_max);
		return;
	}
	point* x_min = (point*)malloc(sizeof(point));
	if (x_min == nullptr) {
		printf("Malloc memory error\n");
		free(x_max);
		free(y_max);
		return;
	}
	point* y_min = (point*)malloc(sizeof(point));
	if (y_min == nullptr) {
		printf("Malloc memory error\n");
		free(x_max);
		free(y_max);
		free(y_min);
		return;
	}
	find_max_min_x_y_points(*values, x_max, y_max, x_min, y_min);

	ss << "graph/test" << segmentid << ".svg";
	errno_t err;
	if ((err = fopen_s(&pFile, ss.str().c_str(), "w")) != 0) {
		printf("Cannot write to segment file\n.");
		return;
	}
	tinyxml2::XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");
	printer.PushAttribute("style", "padding: 40px 25px 15px 50px");
	printer.PushAttribute("width", x_max->x);
	printer.PushAttribute("height", y_max->y + 25);

	print_title(&printer, segmentid, x_max);
	print_axis(&printer, *values, x_max, y_max, y_min, true);
	print_polynate_full_segment(&printer, *values, "rgb(0,0,0)");
	//print_polynate_full_segment(&printer, *values_average, "rgb(0,0,255)");
	
	for (size_t i = peaks_start_index; i < peaks_end_index; i++) {
		print_peaks(&printer, *(peaks.at(i)->peaks), y_max, true);
	}
	
	printer.CloseElement();
	std::cout << printer.CStr();

	free(x_max);
	free(y_max);
	free(x_min);
	free(y_min);
}

/*
	Funkce pro vytvoreni grafu segmentu

	values - vektor obsahujici body segmentu, ktere se maji vykreslit
	points_by_day - vektor obsahujici vsechny segmenty vcetne bodu, ktere jsou rozdelene na jednotlive dny
	point_position - pole obsahujici indexy zacatku jednotlivych segmentu ve vektoru points_by_day
	peaks - vektor obsahujici vsechny vykyvy
	peaks_start_index - index pro prvni vykyv vykreslovaneho segmentu
	peaks_end_index - index pro posledni vykyv vykreslovaneho segmentu
	segmentid - ID segmentu
*/
void SVG::print_graph_split_segment(vector<point*> *points, vector<segment_points*> points_by_day, size_t* point_position, vector<segment_peaks*> peaks, size_t peaks_start_index, size_t peaks_end_index, size_t segmentid) {
	FILE * pFile;
	std::stringstream ss;

	point* x_max = (point*)malloc(sizeof(point));
	if (x_max == nullptr) {
		printf("Malloc memory error\n");
		return;
	}
	x_max->second = 86400;
	x_max->x = 1440;

	point* y_max = (point*)malloc(sizeof(point));
	if (y_max == nullptr) {
		printf("Malloc memory error\n");
		free(x_max);
		return;
	}
	point* y_min = (point*)malloc(sizeof(point));
	if (y_min == nullptr) {
		printf("Malloc memory error\n");
		free(x_max);
		free(y_max);
		return;
	}
	find_max_min_x_y_points(*points, nullptr, y_max, nullptr, y_min);

	size_t days_in_segment = 0, y = 0, temp = *point_position;
	while (*point_position + y < points_by_day.size() && segmentid == points_by_day.at(*point_position + y)->segmentid) {
		days_in_segment++;
		temp++;
		y++;
	}

	ss << "graph/test" << segmentid << ".svg";
	errno_t err;
	if ((err = fopen_s(&pFile, ss.str().c_str(), "w")) != 0) {
		printf("Cannot write to segment file\n.");
		return;
	}
	tinyxml2::XMLPrinter printer(pFile);
	printer.PushDeclaration("xml version=\"1.0\" standalone=\"no\"");
	printer.PushUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
	printer.OpenElement("svg");
	printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
	printer.PushAttribute("version", "1.1");
	printer.PushAttribute("style", "padding: 25px 25px 15px 50px");
	printer.PushAttribute("width", x_max->x);
	printer.PushAttribute("height", (y_max->y + 75) * days_in_segment);


	for (size_t i = 0; i < days_in_segment; i++) {
		create_g_transform(&printer, i * (y_max->y + 65) + 25);
		if (i == 0) {
			print_title(&printer, segmentid, x_max);
		}
		print_axis(&printer, *points, x_max, y_max, y_min, false);
		print_polynate_split_segment(&printer, *(points_by_day.at(*point_position + i)->points), "rgb(0,0,0)");
		
		if (i + peaks_start_index < peaks_end_index) {
			print_peaks(&printer, *(peaks.at(i + peaks_start_index)->peaks), y_max, false);
		}
		printer.CloseElement(); // close G element

	}
	

	printer.CloseElement();
	std::cout << printer.CStr();
	*point_position = temp;

	free(x_max);
	free(y_max);
	free(y_min);
}

/* 
	Vykresleni krivny pro cely segment (jeden graf)

	values - body pro vykresleni
	color - barva krivky
*/
void SVG::print_polynate_full_segment(tinyxml2::XMLPrinter* printer, vector<point*> values, std::string color) {
	std::ostringstream ret_stream;

	for (auto &row : values) {
		ret_stream << row->x << "," << row->y << " ";
	}

	(*printer).OpenElement("polyline");
	(*printer).PushAttribute("points", ret_stream.str().c_str());
	(*printer).PushAttribute("stroke", color.c_str());
	(*printer).PushAttribute("stroke-width", "1.5");
	(*printer).PushAttribute("fill", "transparent");
	(*printer).CloseElement();
}

/*
Vykresleni krivny pro cely segment rozdeleny na jednotlive dny

values - body pro vykresleni
color - barva krivky
*/
void SVG::print_polynate_split_segment(tinyxml2::XMLPrinter* printer, vector<point*> values, std::string color) {
	std::ostringstream ret_stream;

	for (auto &row : values) {
		ret_stream << row->second / SVG::SECOND_IN_MINUTE << "," << row->y << " ";
	}

	(*printer).OpenElement("polyline");
	(*printer).PushAttribute("points", ret_stream.str().c_str());
	(*printer).PushAttribute("stroke", color.c_str());
	(*printer).PushAttribute("stroke-width", "1.5");
	(*printer).PushAttribute("fill", "transparent");
	(*printer).CloseElement();
}

/*
	Zvyrazneni vykyvu v grafu

	peaks - vykyvy v segmentu
	y_max - maximalni Y bod v grafu
	full_graph - podminka, zda vykresleny graf je rozdelen na jednotlive dny v ramci segmentu
*/
void SVG::print_peaks(tinyxml2::XMLPrinter* printer, vector<peak*> peaks, point* y_max, bool full_graph) {
	float x1 = 0;
	float x2 = 0;
	for (auto &value : peaks) {
		if (full_graph) {
			x1 = value->x1->x;
			x2 = value->x2->x;
		}
		else {
			x1 = value->x1->second / (float)SVG::SECOND_IN_MINUTE;
			x2 = value->x2->second / (float)SVG::SECOND_IN_MINUTE;
		}


		(*printer).OpenElement("rect");
		(*printer).PushAttribute("x", x1);
		(*printer).PushAttribute("y", 0);
		(*printer).PushAttribute("width", x2 - x1);
		(*printer).PushAttribute("height", y_max->y + 25);
		(*printer).PushAttribute("style", "fill:green;opacity:0.25");
		(*printer).CloseElement();
	}
}

/*
	Vykresleni ramce okolo grafu
*/
void SVG::print_border(tinyxml2::XMLPrinter* printer) {
	(*printer).OpenElement("rect");
	(*printer).PushAttribute("x", 0);
	(*printer).PushAttribute("y", 0);
	(*printer).PushAttribute("width", "100%");
	(*printer).PushAttribute("height", "100%");
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 2; opacity: 0.5");
	(*printer).CloseElement();
}

/*
	Vykresleni X a Y osy v grafu

	values - body vykreslene v grafu
	x_max - maximalni X bod v grafu
	y_max - maximalni Y bod v grafu
	y_min - minimalni Y bod v grafu
	full_graph - podminka, zda vykresleny graf je rozdelen na jednotlive dny v ramci segmentu
*/
void SVG::print_axis(tinyxml2::XMLPrinter* printer, vector<point*> values, point* x_max, point* y_max, point* y_min, bool full_graph) {
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

	if (full_graph) {
		label_of_axis_full_segment(printer, values, x_max, y_max, y_min);
	}
	else {
		label_of_axis_split_segment(printer, x_max, y_max, y_min);
	}
}

/*
	Popis X a Y os v grafu. Soubor obsahuje pouze jeden graf (cely segment).

	values - body vykreslene v grafu
	x_max - maximalni X bod v grafu
	y_max - maximalni Y bod v grafu
	y_min - minimalni Y bod v grafu
*/
void SVG::label_of_axis_full_segment(tinyxml2::XMLPrinter* printer, vector<point*> values, point* x_max, point* y_max, point* y_min) {
	size_t i = 0;
	for (auto &value : values) {
		if (i % 20 == 0) {
			(*printer).OpenElement("text");
			(*printer).PushAttribute("x", value->x - CENTER_TIME);
			(*printer).PushAttribute("y", y_max->y + TIME_MARGIN);
			(*printer).PushAttribute("fill", "black");
			(*printer).PushAttribute("font-size", "15");
			(*printer).PushText(get_time(value->second).c_str());
			(*printer).CloseElement();

			(*printer).OpenElement("line");
			(*printer).PushAttribute("x1", value->x);
			(*printer).PushAttribute("y1", 0);
			(*printer).PushAttribute("x2", value->x);
			(*printer).PushAttribute("y2", y_max->y);
			(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 1; opacity: 0.5");
			(*printer).CloseElement();
		}
		i++;
	}
	(*printer).OpenElement("line");
	(*printer).PushAttribute("x1", x_max->x);
	(*printer).PushAttribute("y1", 0);
	(*printer).PushAttribute("x2", x_max->x);
	(*printer).PushAttribute("y2", y_max->y);
	(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 1; opacity: 0.5");
	(*printer).CloseElement();

	char number[20];
	float range = (y_min->ist - y_max->ist) / SVG::COUNT_VALUES_Y_AXIS;

	sprintf_s(number, sizeof(number), "%.2f", y_max->ist);
	(*printer).OpenElement("text");
	(*printer).PushAttribute("x", -40);
	(*printer).PushAttribute("y", y_max->y);
	(*printer).PushAttribute("fill", "black");
	(*printer).PushAttribute("font-size", "15");
	(*printer).PushText(number);
	(*printer).CloseElement();
	for (i = 1; i <= SVG::COUNT_VALUES_Y_AXIS; i++) {
		float value = (range * i) + y_max->ist;
		float y = y_max->y / SVG::COUNT_VALUES_Y_AXIS * (SVG::COUNT_VALUES_Y_AXIS - i);
		sprintf_s(number, sizeof(number), "%.2f", value);
		(*printer).OpenElement("text");
		(*printer).PushAttribute("x", -40);
		(*printer).PushAttribute("y", y);
		(*printer).PushAttribute("fill", "black");
		(*printer).PushAttribute("font-size", "15");
		(*printer).PushText(number);
		(*printer).CloseElement();

		(*printer).OpenElement("line");
		(*printer).PushAttribute("x1", 0);
		(*printer).PushAttribute("y1", y);
		(*printer).PushAttribute("x2", x_max->x);
		(*printer).PushAttribute("y2", y);
		(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 1; opacity: 0.5");
		(*printer).CloseElement();
	}
}

/*
	Popis X a Y os v grafu. Soubor obsahuje pouze vice grafu rozdelenych po jednotlivych dnech.

	values - body vykreslene v grafu
	x_max - maximalni X bod v grafu
	y_max - maximalni Y bod v grafu
	y_min - minimalni Y bod v grafu
*/
void SVG::label_of_axis_split_segment(tinyxml2::XMLPrinter* printer, point* x_max, point* y_max, point* y_min) {
	for (int i = 0; i <= 15; i++) {
		int value = i * 96;
		(*printer).OpenElement("text");
		(*printer).PushAttribute("x", value - CENTER_TIME);
		(*printer).PushAttribute("y", y_max->y + TIME_MARGIN);
		(*printer).PushAttribute("fill", "black");
		(*printer).PushAttribute("font-size", "15");
		(*printer).PushText(get_time(value * 60).c_str());
		(*printer).CloseElement();

		(*printer).OpenElement("line");
		(*printer).PushAttribute("x1", value);
		(*printer).PushAttribute("y1", 0);
		(*printer).PushAttribute("x2", value);
		(*printer).PushAttribute("y2", y_max->y);
		(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 1; opacity: 0.5");
		(*printer).CloseElement();
	}

	char number[20];
	float range = (y_min->ist - y_max->ist) / SVG::SVG::COUNT_VALUES_Y_AXIS;

	sprintf_s(number, sizeof(number), "%.2f", y_max->ist);
	(*printer).OpenElement("text");
	(*printer).PushAttribute("x", -40);
	(*printer).PushAttribute("y", y_max->y);
	(*printer).PushAttribute("fill", "black");
	(*printer).PushAttribute("font-size", "15");
	(*printer).PushText(number);
	(*printer).CloseElement();
	for (size_t i = 1; i <= SVG::COUNT_VALUES_Y_AXIS; i++) {
		float value = (range * i) + y_max->ist;
		float y = y_max->y / SVG::COUNT_VALUES_Y_AXIS * (SVG::COUNT_VALUES_Y_AXIS - i);
		sprintf_s(number, sizeof(number), "%.2f", value);
		(*printer).OpenElement("text");
		(*printer).PushAttribute("x", -40);
		(*printer).PushAttribute("y", y);
		(*printer).PushAttribute("fill", "black");
		(*printer).PushAttribute("font-size", "15");
		(*printer).PushText(number);
		(*printer).CloseElement();

		(*printer).OpenElement("line");
		(*printer).PushAttribute("x1", 0);
		(*printer).PushAttribute("y1", y);
		(*printer).PushAttribute("x2", 1440);
		(*printer).PushAttribute("y2", y);
		(*printer).PushAttribute("style", "fill: white; stroke: black; stroke-width: 1; opacity: 0.5");
		(*printer).CloseElement();
	}
}

/*
	Vytvoreni elementu G v souboru

	y - Y hodnota pro transformaci
*/
void SVG::create_g_transform(tinyxml2::XMLPrinter* printer, float y) {
	std::ostringstream ret_stream;
	ret_stream << "translate(0,  " << y << ")";

	(*printer).OpenElement("g");
	(*printer).PushAttribute("transform", ret_stream.str().c_str());
}

/*
	Vykresleni nazvu grafu
*/
void SVG::print_title(tinyxml2::XMLPrinter* printer, size_t segmentid, point* x_max) {
	std::ostringstream ret_stream;
	ret_stream << "Segment " << segmentid;

	(*printer).OpenElement("text");
	(*printer).PushAttribute("x", (x_max->x / 2) - 50);
	(*printer).PushAttribute("y", -15);
	(*printer).PushAttribute("fill", "black");
	(*printer).PushAttribute("font-size", "25");
	(*printer).PushText(ret_stream.str().c_str());
	(*printer).CloseElement();
}