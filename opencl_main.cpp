#include "opencl_main.h"

#define MAX_SOURCE_SIZE (0x100000)

int get_peaks_opencl(int count_point_values, int count_point_average_values, int *count_segments, int* segment_positions,
	float* p_x, float* p_y, float* p_ist, float* pa_x, float* pa_y,
	int* result_count_peaks, float* peak_x1, float* peak_x2, float* peak_sum) {

	// Load the kernel source code into the array source_str
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("peaks_opencl.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
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

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	// Create memory buffers on the device for each vector 
	cl_mem count_segments_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int), NULL, &ret);
	cl_mem segment_positions_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, (*count_segments + 1) * sizeof(int), NULL, &ret);
	cl_mem p_x_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem p_y_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem p_ist_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem pa_x_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, count_point_average_values * sizeof(float), NULL, &ret);
	cl_mem pa_y_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, count_point_average_values * sizeof(float), NULL, &ret);
	
	cl_mem result_count_peaks_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, *count_segments * sizeof(int), NULL, &ret);
	cl_mem peak_x1_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem peak_x2_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, count_point_values * sizeof(float), NULL, &ret);
	cl_mem peak_sum_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, count_point_values * sizeof(float), NULL, &ret);

	// Copy the lists A and B to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, count_segments_mem_obj, CL_TRUE, 0, sizeof(int), count_segments, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, segment_positions_mem_obj, CL_TRUE, 0, (*count_segments + 1) * sizeof(int), segment_positions, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_x_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), p_x, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_y_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), p_y, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_ist_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), p_ist, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, pa_x_mem_obj, CL_TRUE, 0, count_point_average_values * sizeof(float), pa_x, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, pa_y_mem_obj, CL_TRUE, 0, count_point_average_values * sizeof(float), pa_y, 0, NULL, NULL);


	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char **)&source_str, (const size_t *)&source_size, &ret);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	//if (ret != CL_SUCCESS) { // odkomentuj a mas vypis jen kdyz to tam vyfailuje, jinak to vypisuje vse
	char *buff_erro;
	cl_int errcode;
	size_t build_log_len;
	errcode = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
	if (errcode) {
		printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
		exit(-1);
	}

	buff_erro = (char*)malloc(build_log_len);
	if (!buff_erro) {
		printf("malloc failed at line %d\n", __LINE__);
		exit(-2);
	}

	errcode = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, build_log_len, buff_erro, NULL);
	if (errcode) {
		printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
		exit(-3);
	}

	fprintf(stderr, "Build log: \n%s\n", buff_erro); //Be careful with  the fprint
	free(buff_erro);
	fprintf(stderr, "clBuildProgram failed\n");
	//}

	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "calculate_peaks", &ret);

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
	ret = clEnqueueReadBuffer(command_queue, result_count_peaks_mem_obj, CL_TRUE, 0, *count_segments * sizeof(int), result_count_peaks, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, peak_x1_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), peak_x1, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, peak_x2_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), peak_x2, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, peak_sum_mem_obj, CL_TRUE, 0, count_point_values * sizeof(float), peak_sum, 0, NULL, NULL);

	// Clean up
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
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
	ret = clReleaseContext(context);
	free(source_str);

	return 0;
}


int opencl_main_test() {

	// Create the two input vectors
	int i;
	const int LIST_SIZE = 1024;
	int *A = (int*)malloc(sizeof(int)*LIST_SIZE);
	int *B = (int*)malloc(sizeof(int)*LIST_SIZE);
	for (i = 0; i < LIST_SIZE; i++) {
		A[i] = i;
		B[i] = LIST_SIZE - i;
	}

	// Load the kernel source code into the array source_str
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("vector_add_kernel.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
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

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	// Create memory buffers on the device for each vector 
	cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
		LIST_SIZE * sizeof(int), NULL, &ret);
	cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
		LIST_SIZE * sizeof(int), NULL, &ret);
	cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		LIST_SIZE * sizeof(int), NULL, &ret);

	// Copy the lists A and B to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
		LIST_SIZE * sizeof(int), A, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0,
		LIST_SIZE * sizeof(int), B, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char **)&source_str, (const size_t *)&source_size, &ret);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	//if (ret != CL_SUCCESS) { // odkomentuj a mas vypis jen kdyz to tam vyfailuje, jinak to vypisuje vse printf("KOKOT");
	char *buff_erro;
	cl_int errcode;
	size_t build_log_len;
	errcode = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
	if (errcode) {
		printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
		exit(-1);
	}

	buff_erro = (char*)malloc(build_log_len);
	if (!buff_erro) {
		printf("malloc failed at line %d\n", __LINE__);
		exit(-2);
	}

	errcode = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, build_log_len, buff_erro, NULL);
	if (errcode) {
		printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
		exit(-3);
	}

	fprintf(stderr, "Build log: \n%s\n", buff_erro); //Be careful with  the fprint
	free(buff_erro);
	fprintf(stderr, "clBuildProgram failed\n");
	//}

	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);

	// Execute the OpenCL kernel on the list
	size_t global_item_size = LIST_SIZE; // Process the entire lists
	size_t local_item_size = 64; // Divide work items into groups of 64
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);

	// Read the memory buffer C on the device to the local variable C
	int *C = (int*)malloc(sizeof(int)*LIST_SIZE);
	ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0,
		LIST_SIZE * sizeof(int), C, 0, NULL, NULL);

	// Display the result to the screen
	for (i = 0; i < LIST_SIZE; i++)
		printf("%d + %d = %d\n", A[i], B[i], C[i]);

	// Clean up
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(a_mem_obj);
	ret = clReleaseMemObject(b_mem_obj);
	ret = clReleaseMemObject(c_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	free(A);
	free(B);
	free(C);
	return 0;
}