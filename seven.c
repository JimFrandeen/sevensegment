/** @file seven.c
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#define PRINT_DEBUG // Turn this on for debug output

 /*
 * Thinking about how to organize the counters.
 * Each 7-segment counter can be represented by a bit string.
 value  string      abcdefg    #   hex decimal value
 0      abcefg      1110111 6  3   77  119
 1      cf          0010010 2  1   12  018
 2      acdeg       1011101 5  3   5d  093
 3      acdfg       1011011 5  3   58  091
 4      bcdf        0111010 4  1   3a  058
 5      abdfg       1101011 5  3   68  107
 6      abdefg      1101111 6  3   67  103
 7      acf         1010010 3  1   52  082
 8      abcdefg     1111111 7  1   7f  127
 9      abcdfg      1111011 6  3   7b  123
 */
// A segment_mask_t is a byte that contains one of 10 values
typedef unsigned char segment_mask_t; // e.g., abcefg -> 1110111 = 0x77 

/**
* Count the number of segments in a segment_mask
*
* @param    segment_mask_t
* @retval   number of segments
*
*/int count_segments(segment_mask_t segment_mask) {
    int count = 0;
    for (int index = 0; index < 7; index++) {
        if (segment_mask & (1 << index)) count++;
    }
    return count;
}

/**
* Given an array of 10 unique segment masks, map each to a value 0..9
*
* @param    ar_mask an array of 10 unique segment masks
* @param    ar_value an array of 10 values to be returned
*
*/
void map_masks_to_values(segment_mask_t ar_mask[10], unsigned char ar_value[10]) {
    // Save these masks for our calculations. input_mask_7 has 7 segments, ect.
    segment_mask_t input_mask_7, input_mask_4, input_mask6, input_mask8, input_mask9;
#define UNDEFINED_MASK 0xFF
 
    // First pass: Get the values for 1, 4, 7, 8
    for (int mask_index = 0; mask_index < 10; mask_index++) {
        // Set each value to undefined value to be sure we fill in all the values.
        ar_value[mask_index] = UNDEFINED_MASK;

        // Get the next unique mask and segment count from the string of 10.
        segment_mask_t mask = ar_mask[mask_index];
        unsigned char segment_count = count_segments(mask);
        switch (segment_count) {
        case 2:  // 2 segments is 1 
            ar_value[mask_index] = 1; 
            break; 
        case 3:  // 3 segments is 7
            ar_value[mask_index] = 7; 
            input_mask_7 = mask;
            break;
        case 4: // 4 segments is 4
                ar_value[mask_index] = 4; 
                input_mask_4 = mask;
                break;
        case 7:  // 7 segments is 8
                ar_value[mask_index] = 8; 
                input_mask8 = mask;
                break; 
        }
    }
    // Second pass: get the values for 3, 6, 9
    for (int mask_index = 0; mask_index < 10; mask_index++) {
        // Get the next unique mask and segment count from the string of 10.
        segment_mask_t mask = ar_mask[mask_index];
        unsigned char segment_count = count_segments(ar_mask[mask_index]);
        if (segment_count == 5) {
            /* The value is 
            * 2   acdeg 1011101 or
            * 3   acdfg 1011011 or
            * 5   abdfg 1101011
            */
            // If the mask has a '7' in it, it is a 3
            if ((mask & input_mask_7) == input_mask_7) {
                ar_value[mask_index] = 3;
            }
        }
        else if (segment_count == 6) {
            /* The value is
            * 0   abcefg 1110111 or
            * 6   abdefg 1101111 or
            * 9   abcdfg 1111011
            */
            // If the mask does not have a '7' in it, it is a 6
            if ((mask & input_mask_7) != input_mask_7) {
                ar_value[mask_index] = 6;
                input_mask6 = mask;
            }
            // If the mask has a '4' in it, it is a 9
            else if ((mask & input_mask_4) == input_mask_4) {
                ar_value[mask_index] = 9;
                input_mask9 = mask;
            }
            // otherwise, it's a 0
            else {
                ar_value[mask_index] = 0;
            }
        }
    }
    
    // Third pass: get value for 2, 5
    // we have   1, 3, 4, 6, 7, 8, 9
    /* 2 and 5 are missing
     * 6 and 8 differ by one signal: c
     * If we XOR 6 and 8, we will get the bit that represents signal c
     * 2 and 3 have signal 3; 5 does not.
     * 8 and 9 differ by one signal: e
     * 2 has signal e; 3 and 5 do not
     * If we XOR 8 and 9, we will get the bit that represents signal e
     */
    segment_mask_t signal_c = input_mask6 ^ input_mask8;
    segment_mask_t signal_e = input_mask8 ^ input_mask9;
    for (int mask_index = 0; mask_index < 10; mask_index++) {
        segment_mask_t mask = ar_mask[mask_index];
        unsigned char segment_count = count_segments(ar_mask[mask_index]);
        if (segment_count == 5) {
            /* The value is
            * 2   acdeg 1011101 or
            * 3   acdfg 1011011 or
            * 5   abdfg 1101011
            */
            // If the mask does not have signal c, it is 5
            if ((mask & signal_c) != signal_c) {
                ar_value[mask_index] = 5;
            }
            // If the mask has signal e, it is 2
            else if ((mask & signal_e) == signal_e) {
                ar_value[mask_index] = 2;
            }
        }
    }
#ifdef PRINT_DEBUG
    for (int mask_index = 0; mask_index < 10; mask_index++) {
        printf("values: %0d, %0d, %0d, %0d, %0d, %0d, %0d, %0d, %0d, %0d\n", ar_value[0], ar_value[1], ar_value[2], ar_value[3], ar_value[4],
            ar_value[5], ar_value[6], ar_value[7], ar_value[8], ar_value[9]);
    }
#endif
}

/**
* Given a segment mask, an array of 10 unique segment masks, 
* and a parallel array of mask values, return the value of the input segment mask.
*
* @param segment_mask   the segment mask to map to a value
* @param    ar_mask an array of 10 unique segment masks
* @param    ar_value a parallel array of 10 values that correspond to ar_mask
*
*/
int map_segment_to_counter(segment_mask_t segment_mask, segment_mask_t ar_mask[10], unsigned char ar_value[10]) {
    for (int mask_index = 0; mask_index < 10; mask_index++) {
        if (ar_mask[mask_index] == segment_mask)
            return ar_value[mask_index];
    }
    printf("map_segment_to_counter did not find a match for segment_mask %0x\n", segment_mask);
    exit(-1);
}

/**
* Create a segment_mask_t from a string, updating the pointer to the string.
*
* @param    p_input         pointer to pointer to input string
*                           is updated on return to point to the next character.
* @retval   returns segment_mask_t or 0 if end of input.
*
*/
segment_mask_t create_segment_mask_from_string(const char** p_input) {
    // Initialize pattern
    segment_mask_t segment_mask = 0;

    // Scan to the first character of the pattern segment a..g
    while (1) {
        if (*p_input[0] == 0) {
            return 0; // end of input
        }
        // If the next character is not a segment value, increment to the next 
        if ((*p_input[0] < 'a') || (*p_input[0] > 'g')) (*p_input)++;
        else break; // stop when p_input points to first character of segment a..g
    }
    // *p_input[0] points to the segment string. Scan the segment string.
    while (1) {
        // If end of input, we don't increment the pointer so that if we get called again,
        // the pointer will point to the end of of string and fail.
        if (*p_input[0] == 0) break;
        if ((*p_input[0] < 'a') || (*p_input[0] > 'g')) {
            // end of pattern
            (*p_input)++; // point to next character following pattern.
            break;
        }

        // Set the bit in the pattern value that corresponds to this segment
        segment_mask |= 1 << ('g' - *p_input[0]);
        (*p_input)++; // step to next character in pattern
    }
    return segment_mask;
}

/**
* Display "pattern with <n> segments: <pattern 2 to 7 letters a..g>
*
* @param    p_pattern   pointer to segment_mask_t
*/
void show_segment(segment_mask_t segment_mask) {
#ifdef PRINT_DEBUG
    printf("pattern with %d segments: ", count_segments(segment_mask));
    for (int segment_index = 0; segment_index < 8; segment_index++) {
        if (segment_mask & (1 << (6 - segment_index)))
            printf("%c", 'a' + segment_index);
    }
    printf("\n");
#endif
}

/**
* Given a string of input, return the number of unique segments 
* and the total value of all the 4-digit counters.
*   Each entry of the input string consists of ten unique signal patterns, 
*   a | delimiter, and a four digit output value.
*   The string ends with a \0
*
* @param    input   pointer to string in memory.
* @param    p_num_unique_segments_ret pointer to the number of unique segments returned.
* @retval   returns the total value of all the 4-digit counters.
*/
int count_unique_segments(char *input, int *p_num_unique_segments_ret) {
    segment_mask_t segment;
    int num_unique_segments = 0;
    int input_string_number = 0;
    int four_digit_display_value = 0;
    int total_digit_display_value = 0;
    int counter[4];

    // Array of 10 unique input masks in the order received.
    segment_mask_t ar_input_mask[10];

    // Array parallel to ar_input_mask
    unsigned char ar_map_input_mask_to_value[10];

    // Until end of string
    while (1) {
        // Read the 10 unique segments.
        for (int signal_index = 0; signal_index < 10; signal_index++) {
            segment = create_segment_mask_from_string(&input);
            ar_input_mask[signal_index] = segment;
            if (count_segments(segment) == 0)
                // Returns 0 when it detects \0 for end of string
                goto show_counts;
        }
        // Map the 10 segments to 10 values.
        map_masks_to_values(ar_input_mask, ar_map_input_mask_to_value);
        
        // Read the four counters
        for (int counter_index = 0; counter_index < 4; counter_index++) {
            segment = create_segment_mask_from_string(&input);
            // If it's one of the four unique patterns, 
            // increment the counter and display the pattern.
            int num_segments = count_segments(segment);
            if (num_segments == 0) {
                // Returns 0 when it detects \0 for end of string
                printf("Unexpected error reading 4 counters\n");
                exit(-1);
            }
            if ((num_segments == 2)
                || (num_segments == 4)
                || (num_segments == 3)
                || (num_segments == 7)) {
                num_unique_segments++;
                show_segment(segment);
            }
            // Get the value of the current counter.
            counter[counter_index] = map_segment_to_counter(segment, ar_input_mask, ar_map_input_mask_to_value);
        }
        // Get the value of the four-digit display
        four_digit_display_value = 0;
        for (int counter_index = 0; counter_index < 4; counter_index++) {
            int counter_value = counter[counter_index];
            four_digit_display_value = four_digit_display_value * 10 + counter_value;
        }
        // Increment the total counter value
        total_digit_display_value += four_digit_display_value;
        input_string_number++;
    }
show_counts:
    *p_num_unique_segments_ret = num_unique_segments;
    return total_digit_display_value;
}

/**
* Give a file name, open the file of signal pattern entries, read it into memory,
* store a \0 following the last character to signal end of file.
*
* @param    file_name   path to input file.
* @retval   returns pointer to buffer that contains input string
*/
char* read_input(char* file_name) {
    FILE* input_file_handle = fopen(file_name, "rb");
    if (input_file_handle == NULL) {
        fprintf(stderr, "Error reading input_buffer %s: %s\n", file_name, strerror(errno));
        exit(-1);
    }
    // Find out how many bytes are in the file.
    fseek(input_file_handle, 0, SEEK_END);
    size_t file_size = ftell(input_file_handle);
    rewind(input_file_handle);

    // Allocate a buffer to read in the file.
    // Allocate one extra byte so we can store 0 at the end.
    char* input_buffer = (char*)malloc(file_size) + 1;
    if (input_buffer == NULL) {
        printf("Unable to allocate input_buffer buffer %zu bytes\n", file_size);
        exit(-1);
    }

    size_t bytes_read = fread((void*)input_buffer, sizeof(char), file_size, input_file_handle);
    // Make sure we read the whole enchilada.
    if (bytes_read != file_size) {
        printf("Error eof reading %s: %s. Expected %zu bytes, read %zu bytes\n",
            file_name, strerror(errno), bytes_read, file_size);
        exit(-1);
    }
    fclose(input_file_handle);
    input_buffer[file_size] = 0; // Store 0 following the last byte to assure end of string.
    return input_buffer;
}

int main(int argc, char* argv[]) {
    // For debugging, see if we get the expected counter value for the first example.
    int num_unique_segments;
    int counter = count_unique_segments("acedgfb cdfbe gcdfa fbcad dab cefabd cdfgeb eafb cagedb ab | cdfeb fcadb cdfeb cdbaf",
        &num_unique_segments);

    // Read the example 
    char *input_buffer = read_input("example.txt");
    
    // Count and display the number of unique segments in example.txt.
    int counter_total = count_unique_segments(input_buffer, &num_unique_segments);
    printf("Number of unique segments for example.txt: %d, Total counter value: %d\n",
        num_unique_segments, counter_total);

    // Read the puzzle input.
    input_buffer = read_input("puzzle_input.txt");

    // Count and display the number of unique segments in puzzle_input.txt.
    counter_total = count_unique_segments(input_buffer, &num_unique_segments);
    printf("Number of unique segments for puzzle_input.txt: %d, Total counter value: %d\n",
        num_unique_segments, counter_total);
    return 0;
}
