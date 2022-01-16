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
 0   abcefg      1110111 6  3   77  119
 1   cf          0010010 2  1   12  018
 2   acdeg       1011101 5  3   5d  093
 3   acdfg       1011011 5  3   58  091
 4   bcdf        0111010 4  1   3a  058
 5   abdfg       1101011 5  3   68  107
 6   abdefg      1101111 6  3   67  103
 7   acf         1010010 3  1   52  082
 8   abcdefg     1111111 7  1   7f  127
 9   abcdfg      1111011 6  3   7b  123
 */

typedef unsigned char segment_mask_t; // e.g., abcefg -> 1110111 -> 0x77 -> 119

// Given the index of a signal, value is output segment mask.
// e.g., index 0 has 1110111
segment_mask_t rg_signal_to_output_mask[10];

// Array of 10 unique input masks in the order received.
segment_mask_t rg_input_mask[10];

// For each input signal, the mask of the output signal.
// e.g., input signal[0] for input signal 'g', 
// and value is the bit position in the output.
segment_mask_t rg_input_signal_to_output_signal[7];

/**
* Given a segment_mask_t and a pointer to an array of segment masks, return the index in the array.
*
* @param    segment_mask segment mask to match
* @param    p_rg_segment_mask points to array of 10 segment masks.
* @retval   returns index 0..0
*
*/
int index_from_segment_mask(segment_mask_t segment_mask, segment_mask_t* p_rg_segment_mask) {
    for (int index = 0; index < 10; index++)
        if (segment_mask == p_rg_segment_mask[index]) return index;
    printf("segment mask %0x not found in segment mask array\n");
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
* Display "pattern with <n> segments: <pattern 2 to 7 letters a..g>
*
* @param    p_pattern   pointer to segment_mask_t
*/
void show_segment(segment_mask_t segment_mask) {
    printf("pattern with %d segments: ", count_segments(segment_mask));
    for (int segment_index = 0; segment_index < 8; segment_index++) {
        if (segment_mask & (1 << (6 - segment_index)))
            printf("%c", 'a' + segment_index);
    }
    printf("\n");
}

/**
* Create array of 10 signal masks from a string of 10 unique signals
*
* @param    p_input_string   pointer to pointer to string that contains 10 segments
*           point to string gets increments to point to next character following last segment.
* @param    p_rg_segment_array   pointer to array of 10 segment_mask_t
*/
static void create_signal_mask_array_from_string(char** p_input_string, segment_mask_t* p_rg_segment_array) {
    for (int pattern_index = 0; pattern_index < 10; pattern_index++) {
        p_rg_segment_array[pattern_index] = create_segment_mask_from_string(p_input_string);
        show_segment(p_rg_segment_array[pattern_index]);
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
    segment_mask_t segment;
    int num_unique_counters = 0;
    // Until end of string
    while (1) {
        // Discard the 10 unique segments.
        for (int count = 0; count < 10; count++) {
            segment = create_segment_mask_from_string(&input);
            if (count_segments(segment) == 0)
                // Returns 0 when it detects \0 for end of string
                goto show_counts;
        }
        // Read the four counters
        for (int count = 0; count < 4; count++) {
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
                num_unique_counters++;
                show_segment(segment);
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

/*
* Thinking about the next problem.
* Find input2 with 2 signals.   ab 1100000 This will map to segment2  cf: 0010010
* Find input3 with 3 signals.  dab 1101000 This will map to segment3 acf: 1010010
* input2 and input3 have 2 signals in common and two segments in common.
* We can see signals and segments in common by ANDing them:
*         signals in common        1100000       segments in common       0010010
* If we calculate ~input2 & input3 we will get the one signal that is different.
*                  ~input2         0011111
*                   input3         1101000
*         ~input2 & input3         0001000   signal that is different
* If we calculate ~segment2 & segment3 we will get the one segment that is different.
*                 ~segment2        1101101
*                  segment3        1010010
*       ~segment2 & segment3             segment that is different
* This gives our first mapping:
* signal d maps to segment a       0001000                                1000000
*
* Step 2: We have looked at signal2 with 2 signals and signal3 if 3 signals
if we remove acf from every other segment, what is left
0   abcefg      1110111 6  3   77  119
1
2   acdeg       1011101 5  3   5d  093
3   acdfg       1011011 5  3   58  091
5   abdfg       1101011 5  3   68  107
6   abdefg      1101111 6  3   67  103
7
8   abcdefg     1111111 7  1   7f  127
9   abcdfg      1111011 6  3   7b  123
        acf     1010010
       ~acf     0101101
                0000001 g
if we remove acf from every other signal, what is left
acedgfb 1111111
cdfbe   0111110
gcdfa   1011011
fbcad   1111010
cefabd  1111110
cdfgeb  0111111
cagedb  1111101
abd     1101000
~abd    0010111
        0010000  AhHa! signal c 0010000 maps to segment g 0000001
                       signal d 0001000 maps to segment a 1000000                                1000000
Step 3:
* The three segments have 3 bits in common
* 2   acdeg       1011101
* 3   acdfg       1011011
* 5   abdfg       1101011
*                 1001001 we already have a and g. That leaves d 0001000

* Find the 3 signals with five signals and AND them together
*  fdcge   0011111
*  fecdb   0111110
*  fabcd   1111010
           0011010 we already have c and d. That leaves f 0000010
                   f 0000010 maps to d 0001000
                       signal c 0010000 maps to segment g 0000001
                       signal d 0001000 maps to segment a 1000000                                1000000
                       signal f 0000010 maps to segment d 0001000
                                0011010 segments we have
                                1100101 ~segments we have
Step 4:
The three segments with six signals have 4 bits in common
0   abcefg      1110111 6  3   77  119
6   abdefg      1101111 6  3   67  103
9   abcdfg      1111011 6  3   7b  123
                1100011 AND of all the bits
                1001001 we have bits adg
                0110110 ~adg
                0000010 then e is left 0000010

find the three signals with six signals
acedgfb cdfbe gcdfa fbcad dab cefabd cdfgeb eafb cagedb ab
cefabd 1111110
cdfgeb 0111111
cagedb 1111101
       0111100 AND of all the bits
        we have bits

if we remove acfg from every other segment, what is left
0   abcefg      1110111 6  3   77  119
2   acdeg       1011101 5  3   5d  093
3   acdfg       1011011 5  3   58  091
5   abdfg       1101011 5  3   68  107
6   abdefg      1101111 6  3   67  103
8   abcdefg     1111111 7  1   7f  127
9   abcdfg      1111011 6  3   7b  123
        acfg    1010011
       ~acfg    0101100

if we remove acfg from every other signal, what is left
acedgfb 1111111
cdfbe   0111110
gcdfa   1011011
fbcad   1111010
cefabd  1111110
cdfgeb  0111111
cagedb  1111101
abd     1101000
~abd    0010111

*/
/**
* Give a pointer to an input string, create rg_input_mask
*
* @param    file_name   path to input file.
*/
create_signal_map(char** p_input, segment_mask_t* p_mask_ret) {

}
char* counter_signals = "abcefg cf acdeg acdfg bcdf abdfg abdefg acf abcdefg abcdfg";
int main(int argc, char* argv[]) {
    char* p_counter_signals = counter_signals;
    create_signal_mask_array_from_string(&p_counter_signals, rg_signal_to_output_mask);
    count_puzzle_unique_segments("example.txt");
    count_puzzle_unique_segments("puzzle_input.txt");
    return 0;
}
