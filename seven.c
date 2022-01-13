/** @file seven.c
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/*
* Thinking about how to organize the counters. 
* Each counter can be represented by a bit string.
                abcdefg    #   hex decimal value
0   abcefg      1110111 6  2   77  119
1   cf          0010010 2  1   12  018
2   acdeg       1011101 5  4   5d  093
3   acdfg       1011011 5  4   58  091  
4   bcdf        0111010 4  1   3a  058
5   abdfg       1101011 5  4   68  107
6   abdefg      1100111 5  4   67  103
7   acf         1010010 3  1   52  082
8   abcdefg     1111111 7  1   7f  127
9   abcdfg      1111011 6  2   7b  123
*/

typedef struct {
    uint8_t value; // e.g., abcefg -> 1110111 -> 0x77 -> 119
    uint8_t num_segments;
} signal_pattern_t;

// This array will have 10 possible signal patterns.
static signal_pattern_t signal_pattern_array[10];

/**
* Create a signal_pattern_t from a pointer to a string.
*
* @param    p_input         pointer to pointer to input string
*                           is updated on return to point to the next character.
* @param    p_pattern_ret   pointer to signal_pattern_t returned
* @retval   0               success
* @retval   1               fail
*
*/
int create_pattern(const char** p_input, signal_pattern_t* p_pattern_ret) {
    // Initialize pattern
    p_pattern_ret->value = p_pattern_ret->num_segments = 0;

    // Scan to the first character of the pattern segment a..g
    while (1) {
        if (*p_input[0] == 0) {
            printf("End of input unexpected\n");
            return 1; // fail if end of input
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
        // p_input points to next character in pattern segment a..g
        // Increment the number of segments for this pattern.
        p_pattern_ret->num_segments++;
        if (p_pattern_ret->num_segments > 7) { // sanity check
            printf("ERROR: not expecting more than 7 segments\n");
            return 1;
        }

        // Set the bit in the pattern value that corresponds to this segment
        p_pattern_ret->value |= 1 << ('g' - *p_input[0]);
        (*p_input)++; // step to next character in pattern
    }
    return 0; // success
}

/**
* Display "pattern with <n> segments: <pattern 2 to 7 letters a..g>
*
* @param    p_pattern   pointer to signal_pattern_t
*/
void show_pattern(signal_pattern_t* p_pattern) {
    printf("pattern with %d segments: ", p_pattern->num_segments);
    for (int segment_index = 0; segment_index < 8; segment_index++) {
        if (p_pattern->value & (1 << (6 - segment_index)))
            printf("%c", 'a' + segment_index);
    }
    printf("\n");
}

/**
* Initialize an array of 10 patterns
*
* @param    p_signal_pattern_array   pointer to array of signal_pattern_t
*/
static void initialize_patterns(signal_pattern_t* p_signal_pattern_array) {
    char* p_pattern = "abcefg cf acdeg acdfg bcdf abdfg abdefg acf abcdefg abcdfg";
    for (int pattern_index = 0; pattern_index < 10; pattern_index++) {
        create_pattern(&p_pattern, &p_signal_pattern_array[pattern_index]);
        show_pattern(&p_signal_pattern_array[pattern_index]);
    }
}
/**
* Count the number of unique segments from a string of input.
*
* @param    input   pointer to string in memory. 
*   Each entry consists of ten unique signal patterns, a | delimiter, 
*   and a four digit output value.
*   The string ends with a \0
*/
void count_unique_segments(char* input) {
    signal_pattern_t segment;
    int num_unique_counters = 0;
    // Until end of string
    while (1) {
        // Discard the 10 unique segments.
        for (int count = 0; count < 10; count++) {
            if (create_pattern(&input, &segment)) 
                // Returns 1 when it detects \0 for end of string
                goto show_counts; 
        }
        // Read the four counters
        for (int count = 0; count < 4; count++) {
            if (create_pattern(&input, &segment)) {
                printf("Unexpected error reading 4 counters\n");
                exit(-1);
            }
            // If it's one of the four unique patterns, 
            // increment the counter and display the pattern.
            if ((segment.num_segments == 2)
                || (segment.num_segments == 4)
                || (segment.num_segments == 3)
                || (segment.num_segments == 7)) {
                num_unique_counters++;
                show_pattern(&segment);
            }
        }
    }
show_counts:
    printf("%d unique counters found\n", num_unique_counters);
}

/**
* Give a file name, open the file of signal pattern entries,
* open the file, read it into memory, 
* store a \0 following the last character to signal end of file.
* Count the number of unique segments.
*
* @param    file_name   path to input file.
*/
void count_puzzle_unique_segments(char* file_name) {
    FILE* input_file_handle = fopen(file_name, "rb");
    if (input_file_handle == NULL) {
        fprintf(stderr, "Error reading input_buffer %s: %s\n", file_name, strerror(errno));
        exit(-1);
    }
    // Find out how many bytes are in the file.
    fseek(input_file_handle, 0, SEEK_END);
    unsigned file_size = ftell(input_file_handle);
    rewind(input_file_handle);

    // Allocate a buffer to read in the file.
    // Allocate one extra byte so we can store 0 at the end.
    char* input_buffer = (char*)malloc(file_size) + 1; 
    if (input_buffer == NULL) {
        printf("Unable to allocate input_buffer buffer %d bytes\n", file_size);
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

    // Count and display the number of unique segments.
    count_unique_segments(input_buffer);
}

int main(int argc, char* argv[]) {
    initialize_patterns(signal_pattern_array);
    count_puzzle_unique_segments("example.txt");
    count_puzzle_unique_segments("puzzle_input.txt");
    return 0;
}