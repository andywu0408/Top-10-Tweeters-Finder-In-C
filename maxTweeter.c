#include <stdlib.h>
#include <stdio.h>

#define FILE_LINES_MAX = 20000
#define LINE_CHARS_MAX = 1024

int main (int argc, char* argv[]) {

	// Check for valid program input
	// Must have 1 command line argument (filename)
	if (argc != 2) {
		printf("Invalid Input Format");
		exit(1); // unsuccessful exit
	}

	// Store and open file
	char* file_name = argv[1];
	FILE* file_ptr = fopen(file_name, "r");

	// Check if file exists
	if (file_ptr == NULL) {
		printf("Invalid Input Format");
		exit(1); // unsuccessful exit
	}

}

