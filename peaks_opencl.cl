__kernel void calculate_peaks(__global const int *count_segments, __global const int* segment_positions,
	__global const float* p_x, __global const float* p_y, __global const float* p_ist,
	__global const float* pa_x, __global const float* pa_y,
	__global int* result_count_peaks, __global size_t* peak_x1, __global size_t* peak_x2, __global float* peak_sum) {
 
	const int MOVING_AVERAGE = 21;
	const int MIN_MINUTE_FOR_ACTION = 30;

	int id, segments, start_position, start_position_average, i, y = 0, count_points_of_segment, count_points_average_of_segment, count_peaks = 0, temp_peak_x_index = 0;
	float temp_p_x, temp_p_y, temp_p_ist, temp_pa_x, temp_pa_y, sum = 0, grow = 0, temp_peak_x, temp_peak_ist, temp_sum = 0;
	bool is_peak = false;

	id = get_global_id(0);
	segments = *count_segments;

	if (id < segments) {
		count_points_of_segment = segment_positions[id + 1] - segment_positions[id];
		count_points_average_of_segment = count_points_of_segment - MOVING_AVERAGE + 1;

		start_position = segment_positions[id];
		start_position_average = id == 0 ? 0 : start_position - ((MOVING_AVERAGE - 1) * id);

		for (i = 0; i < count_points_of_segment; i++) {

			if (y >= count_points_average_of_segment) { // i je >= nez celkova velikost average vectoru, tak konec cyklu (uz neni kam sahat)
				break;
			}

			temp_p_x = p_x[start_position + i];
			temp_p_y = p_y[start_position + i];
			temp_p_ist = p_ist[start_position + i];
			temp_pa_x = pa_x[start_position_average + y];
			temp_pa_y = pa_y[start_position_average + y];

			if (temp_p_x != temp_pa_x) { // tato podminka je kvuli zacatku points vektoru, protoze obsahuje body, ktery points_average neobsahuje
				continue;
			}

			if (temp_p_y <= temp_pa_y) {
				if (!is_peak) {
					temp_peak_x_index = i;
					temp_peak_x = temp_p_x;
					temp_peak_ist = temp_p_ist;
					sum = 0;
					grow = 0;
					is_peak = true;
				}
				else {
					temp_sum = temp_peak_ist - temp_p_ist;
					temp_sum = temp_sum < 0 ? temp_sum * (-1) : temp_sum;
					sum += temp_sum;
					grow += temp_peak_ist - temp_p_ist;
				}
			}
			else {
				if (is_peak) {
					if (temp_p_x - temp_peak_x >= MIN_MINUTE_FOR_ACTION && grow < 3) {
						peak_x1[start_position + count_peaks] = temp_peak_x_index;
						peak_x2[start_position + count_peaks] = i;
						peak_sum[start_position + count_peaks] = sum;
						count_peaks++;
					}
					is_peak = false;
				}
			}
			y++;
		}
		result_count_peaks[id] = count_peaks;
	}

}