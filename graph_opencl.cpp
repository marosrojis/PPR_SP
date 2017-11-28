#include "graph_opencl.h"

using namespace std;

/*
	Komparator pro serazeni vektoru vykyvu
*/
bool comparatorBySum(const peak* a, const peak* b) {
	return a->sum > b->sum;
}

/*
	Funkce pro inicializaci vsech potrebnych promennych pro praci s GPU
*/
cl_config* prepare_opencl_config() {
	cl_config* config = (cl_config*) malloc(sizeof(cl_config));
	if (config == nullptr) {
		printf("Malloc memory error\n");
		return nullptr;
	}

	// Load the kernel source code into the array source_str
	FILE *fp;
	char *source_str;
	size_t source_size;
	errno_t err;

	if ((err = fopen_s(&fp, "peaks_opencl.cl", "r")) != 0) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	if (source_str == nullptr) {
		printf("Malloc memory error\n");
		return nullptr;
	}
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	// Get platform and device information
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1,
		&device_id, &ret_num_devices);

	// Create an OpenCL context
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char **)&source_str, (const size_t *)&source_size, &ret);

	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "calculate_peaks", &ret);

	config->context = context;
	config->device_id = device_id;
	config->kernel = kernel;
	config->platform_id = platform_id;
	config->program = program;

	free(source_str);

	return config;
}

/*
	Funkce pro zavolani vypoctu vsech vykyvu na graficke karte

	config - inicializovane promenne pro praci s GPU
	count_point_values - pocet vsech bodu ze vsech segmentu
	count_point_average_values - pocet vsech bodu ze vsech segmentu (klouzavy prumer)
	count_segments - pocet segmentu
	segment_positions - pozice zacatku bodu pro vsechny segmentu
	p_x - ukazatel na pole vsech X hodnot
	p_y - ukazatel na pole vsech Y hodnot
	p_ist - ukazatel na pole vsech IST hodnot
	pa_x - ukazatel na pole vsech X hodnot (klouzavy prumer)
	pa_y - ukazatel na pole vsech Y hodnot (klouzavy prumer)
	result_count_peaks - alokovane pole pro pocet vykyvu v segmentech
	peak_x1 - index X1 bodu znacici zacatek vykyvu
	peak_x2 - index X2 bodu znacici konec vykyvu
	peak_sum - ohodnoceni vsech vykyvu
*/
size_t do_opencl_peaks(cl_config* config, size_t count_point_values, size_t count_point_average_values, size_t *count_segments, size_t* segment_positions,
	float* p_x, float* p_y, float* p_ist, float* pa_x, float* pa_y,
	size_t* result_count_peaks, size_t* peak_x1, size_t* peak_x2, float* peak_sum) {

	cl_int ret;
	
	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(config->context, config->device_id, 0, &ret);

	// Create memory buffers on the device for each vector 
	cl_mem count_segments_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, sizeof(size_t), NULL, &ret);
	cl_mem segment_positions_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, (*count_segments + 1) * sizeof(size_t), NULL, &ret);
	cl_mem p_x_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem p_y_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem p_ist_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem pa_x_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, count_point_average_values * sizeof(float), NULL, &ret);
	cl_mem pa_y_mem_obj = clCreateBuffer(config->context, CL_MEM_READ_ONLY, count_point_average_values * sizeof(float), NULL, &ret);

	cl_mem result_count_peaks_mem_obj = clCreateBuffer(config->context, CL_MEM_WRITE_ONLY, *count_segments * sizeof(size_t), NULL, &ret);
	cl_mem peak_x1_mem_obj = clCreateBuffer(config->context, CL_MEM_WRITE_ONLY, count_point_values * sizeof(size_t), NULL, &ret);
	cl_mem peak_x2_mem_obj = clCreateBuffer(config->context, CL_MEM_WRITE_ONLY, count_point_values * sizeof(size_t), NULL, &ret);
	cl_mem peak_sum_mem_obj = clCreateBuffer(config->context, CL_MEM_WRITE_ONLY, count_point_values * sizeof(float), NULL, &ret);

	// Copy the lists A and B to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, count_segments_mem_obj, CL_TRUE, 0, sizeof(size_t), count_segments, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, segment_positions_mem_obj, CL_TRUE, 0, (*count_segments + 1) * sizeof(size_t), segment_positions, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_x_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), p_x, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_y_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), p_y, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_ist_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), p_ist, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, pa_x_mem_obj, CL_TRUE, 0, count_point_average_values * sizeof(float), pa_x, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, pa_y_mem_obj, CL_TRUE, 0, count_point_average_values * sizeof(float), pa_y, 0, NULL, NULL);

	// Build the program
	ret = clBuildProgram(config->program, 1, &(config->device_id), NULL, NULL, NULL);

	if (ret != CL_SUCCESS) { // odkomentuj a mas vypis jen kdyz to tam vyfailuje, jinak to vypisuje vse
		char *buff_erro;
		cl_int errcode;
		size_t build_log_len;
		errcode = clGetProgramBuildInfo(config->program, config->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
		if (errcode) {
			printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
			exit(-1);
		}

		buff_erro = (char*)malloc(build_log_len);
		if (!buff_erro) {
			printf("malloc failed at line %d\n", __LINE__);
			exit(-2);
		}

		errcode = clGetProgramBuildInfo(config->program, config->device_id, CL_PROGRAM_BUILD_LOG, build_log_len, buff_erro, NULL);
		if (errcode) {
			printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
			exit(-3);
		}

		fprintf(stderr, "Build log: \n%s\n", buff_erro); //Be careful with  the fprint
		free(buff_erro);
		fprintf(stderr, "clBuildProgram failed\n");
	}

	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(config->program, "calculate_peaks", &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&count_segments_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&segment_positions_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&p_x_mem_obj);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&p_y_mem_obj);
	ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&p_ist_mem_obj);
	ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&pa_x_mem_obj);
	ret = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *)&pa_y_mem_obj);
	ret = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&result_count_peaks_mem_obj);
	ret = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void *)&peak_x1_mem_obj);
	ret = clSetKernelArg(kernel, 9, sizeof(cl_mem), (void *)&peak_x2_mem_obj);
	ret = clSetKernelArg(kernel, 10, sizeof(cl_mem), (void *)&peak_sum_mem_obj);

	// Execute the OpenCL kernel on the list
	size_t global_item_size = 80; // Process the entire lists
	size_t local_item_size = 1; // Divide work items into groups of 64
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);

	// Read the memory buffer C on the device to the local variable C
	ret = clEnqueueReadBuffer(command_queue, result_count_peaks_mem_obj, CL_TRUE, 0, *count_segments * sizeof(size_t), result_count_peaks, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, peak_x1_mem_obj, CL_TRUE, 0, count_point_values * sizeof(size_t), peak_x1, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, peak_x2_mem_obj, CL_TRUE, 0, count_point_values * sizeof(size_t), peak_x2, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, peak_sum_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), peak_sum, 0, NULL, NULL);

	// Clean up
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(config->kernel);
	ret = clReleaseProgram(config->program);
	ret = clReleaseMemObject(count_segments_mem_obj);
	ret = clReleaseMemObject(segment_positions_mem_obj);
	ret = clReleaseMemObject(p_x_mem_obj);
	ret = clReleaseMemObject(p_y_mem_obj);
	ret = clReleaseMemObject(p_ist_mem_obj);
	ret = clReleaseMemObject(pa_x_mem_obj);
	ret = clReleaseMemObject(pa_y_mem_obj);
	ret = clReleaseMemObject(result_count_peaks_mem_obj);
	ret = clReleaseMemObject(peak_x1_mem_obj);
	ret = clReleaseMemObject(peak_x2_mem_obj);
	ret = clReleaseMemObject(peak_sum_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(config->context);

	return 0;
}

/*
	Funkce pro vytvoreni pole zacatku pozic bodu jednotlivych segmentu

	points - vektor obsahujici vsechny segmenty vcetne bodu
*/
size_t* get_positions_of_points(vector<segment_points*> points) {
	size_t malloc_size = points.size() + 1;
	size_t* segment_positions = (size_t*)malloc(sizeof(size_t) * malloc_size);
	if (segment_positions == nullptr) {
		printf("Malloc memory error\n");
		return nullptr;
	}
	
	size_t position = 0;
	for (size_t i = 0; i < malloc_size; i++) {
		if (i == 0) {
			segment_positions[i] = 0;
		}
		else {
			segment_points* segment = points.at(i - 1);
			position += segment->points->size();
			segment_positions[i] = position;
		}
	}

	return segment_positions;
}

/*
	Funkce pro prevedeni vektoru segmentu do poli primitivnich datavych typu, ktery slouzi pro predani GPU
	
	points - vektor obsahujici vsechny segmenty vcetne bodu
	x_values - preadlokovanane pole pro X hodnoty
	y_values - preadlokovanane pole pro Y hodnoty
	ist_values - preadlokovanane pole pro IST hodnoty
	average - podminka, zda se jedna o vektor segmentu klouzaveho prumeru
*/
void get_segments_info(vector<segment_points*> points, float* x_values, float* y_values, float* ist_values, bool average) {
	size_t z = 0;
	for (size_t i = 0; i < points.size(); i++) {
		segment_points* segment = points.at(i);
		
		for (size_t y = 0; y < segment->points->size(); y++) {
			x_values[z] = segment->points->at(y)->x;
			y_values[z] = segment->points->at(y)->y;
			
			if (!average) {
				ist_values[z] = segment->points->at(y)->ist;
			}
			z++;
		}
	}
}

/*
	Funkce pro vytvoreni vektoru vykyvu ziskanych z GPU

	points - vektor obsahujici vsechny segmenty vcetne bodu
	points_by_day - vektor obsahujici vsechny segmenty vcetne bodu, ktere jsou rozdelene na jednotlive dny
	result_count_peaks - pocet nalezenych vykyvu vsech segmentu
	peak_x1 - index X1 bodu znacici zacatek vykyvu
	peak_x2 - index X2 bodu znacici konec vykyvu
	peak_sum - ohodnoceni vsech vykyvu
	segment_positions - pozice zacatku segmentu v poli obsahujici vsechny body
	result_segments_position - pole obsahujici indexy pocatku kazdeho segmentu
*/
vector<segment_peaks*> create_peaks(vector<segment_points*> points, vector<segment_points*> points_by_day, 
	size_t* result_count_peaks, size_t* peak_x1, size_t* peak_x2, float* peak_sum, size_t* segment_positions, size_t** result_segments_position) {
	vector<segment_peaks*> results;
	size_t seg_day = 0;

	for (size_t i = 0; i < points.size(); i++) {
		vector<peak*> peaks;
		for (size_t y = 0; y < result_count_peaks[i]; y++) {
			peak* tmp_peak = (peak*)malloc(sizeof(peak));
			if (tmp_peak == nullptr) {
				printf("Malloc memory error\n");
				return results;
			}

			tmp_peak->x1 = points.at(i)->points->at(peak_x1[segment_positions[i] + y]);
			tmp_peak->x2 = points.at(i)->points->at(peak_x2[segment_positions[i] + y]);
			tmp_peak->sum = peak_sum[segment_positions[i] + y];
			peaks.push_back(tmp_peak);
		}
		
		(*result_segments_position)[i] = results.size();
		while (points.at(i)->segmentid != points_by_day.at(seg_day)->segmentid) {
			seg_day++;
		}
		vector<peak*> peaks_in_day;
		for (auto &peak_seg : peaks) {
			float start_day = points_by_day.at(seg_day)->points->at(0)->x;
			float end_day = points_by_day.at(seg_day)->points->at(points_by_day.at(seg_day)->points->size() - 1)->x;
			if (peak_seg->x1->x >= start_day && peak_seg->x1->x <= end_day) {
				peaks_in_day.push_back(peak_seg);
			}
			else if (peaks_in_day.size() != 0) {
				segment_peaks* seg_peaks = create_segment_peaks(&peaks_in_day, points.at(i)->segmentid);
				peaks_in_day.clear();
				peaks_in_day.push_back(peak_seg);
				results.push_back(seg_peaks);
				seg_day++;
			}
			else {
				peaks_in_day.push_back(peak_seg);
				seg_day++;
			}
		}

		if (peaks_in_day.size() != 0) {
			segment_peaks* seg_peaks = create_segment_peaks(&peaks_in_day, points.at(i)->segmentid);
			results.push_back(seg_peaks);
		}
	}

	return results;
}

/*
	Funkce pro ziskani vsech vykyvu pomoci GPU

	config - inicializovane promenne pro praci s GPU
	points - vektor obsahujici vsechny segmenty a jejich body
	points_average - vektor obsahujici vsechny segmenty a body vytvorene pomoci klouzaveho prumeru
	points_by_day - vektor obsahujici vsechny segmenty vcetne bodu, ktere jsou rozdelene na jednotlive dny
	result_segments_position - pole obsahujici indexy pocatku kazdeho segmentu
*/
vector<segment_peaks*> get_peaks_opencl(cl_config* config, vector<segment_points*> points, vector<segment_points*> points_average, vector<segment_points*> points_by_day, size_t** result_segments_position) {
	vector<segment_peaks*> peaks;
	size_t count_segments = points.size();
	size_t* segment_positions = get_positions_of_points(points);
	size_t count_point_values = segment_positions[count_segments];
	size_t count_point_average_values = count_point_values - MOVING_AVERAGE + 1;

	size_t* p_count_segments = (size_t*)malloc(sizeof(size_t));
	if (p_count_segments == nullptr) {
		printf("Malloc memory error\n");
		return peaks;
	}

	*p_count_segments = count_segments;
	float* p_x = (float*)malloc(sizeof(float) * count_point_values);
	if (p_x == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		return peaks;
	}
	float* p_y = (float*)malloc(sizeof(float) * count_point_values);
	if (p_y == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		return peaks;
	}
	float* p_ist = (float*)malloc(sizeof(float) * count_point_values);
	if (p_ist == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		return peaks;
	}

	float* pa_x = (float*)malloc(sizeof(float) * count_point_average_values);
	if (pa_x == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		free(p_ist);
		return peaks;
	}
	float* pa_y = (float*)malloc(sizeof(float) * count_point_average_values);
	if (pa_y == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		free(p_ist);
		free(pa_x);
		return peaks;
	}

	get_segments_info(points, p_x, p_y, p_ist, false);
	get_segments_info(points_average, pa_x, pa_y, nullptr, true);

	size_t* result_count_peaks = (size_t*)malloc(sizeof(size_t) * count_segments);
	if (result_count_peaks == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		free(p_ist);
		free(pa_x);
		free(pa_y);
		return peaks;
	}
	size_t* peak_x1 = (size_t*)malloc(sizeof(size_t) * count_point_values);
	if (peak_x1 == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		free(p_ist);
		free(pa_x);
		free(pa_y);
		free(result_count_peaks);
		return peaks;
	}
	size_t* peak_x2 = (size_t*)malloc(sizeof(size_t) * count_point_values);
	if (peak_x2 == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		free(p_ist);
		free(pa_x);
		free(pa_y);
		free(result_count_peaks);
		free(peak_x1);
		return peaks;
	}
	float* peak_sum = (float*)malloc(sizeof(float) * count_point_values);
	if (peak_sum == nullptr) {
		printf("Malloc memory error\n");
		free(p_count_segments);
		free(p_x);
		free(p_y);
		free(p_ist);
		free(pa_x);
		free(pa_y);
		free(result_count_peaks);
		free(peak_x1);
		free(peak_x2);
		return peaks;
	}

	do_opencl_peaks(config, count_point_values, count_point_average_values, p_count_segments, segment_positions, p_x, p_y, p_ist, pa_x, pa_y, result_count_peaks, peak_x1, peak_x2, peak_sum);

	peaks = create_peaks(points, points_by_day, result_count_peaks, peak_x1, peak_x2, peak_sum, segment_positions, result_segments_position);

	free(segment_positions);
	free(p_count_segments);
	free(p_x);
	free(p_y);
	free(p_ist);
	free(pa_x);
	free(pa_y);
	free(result_count_peaks);
	free(peak_x1);
	free(peak_x2);
	free(peak_sum);

	return peaks;
}