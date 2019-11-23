#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_LINES_MAX = 20000
#define LINE_CHARS_MAX = 1024

int getNameColumn (char* first_line);

int main (int argc, char* argv[]) {

	// Check for valid program input
	// Must have 1 command line argument (filename)
	if (argc != 2) {
		printf("Invalid Input Format");
		exit(1); // unsuccessful exit
	}

	// Store and open file
	const char* file_name = argv[1];
	FILE* file_ptr = fopen(file_name, "r");

	// Check if file exists
	if (file_ptr == NULL) {
		printf("Invalid Input Format");
		exit(1); // unsuccessful exit
	}

	// Store first line 
	char first_line[1024];
	fgets(first_line, 1024, file_ptr);

	// Get column position of "name" 
	int num_col = getNameColumn(first_line);
	// printf("%d\n", num_col);

}

int getNameColumn (char* first_line) {
	
	int num_col = 0;
	// Check if first line exists
	if (first_line == NULL) {
		printf("Invalid Input Format");
		exit(1); // unsuccessful exit
	}
	else {
		// Search first line for "name" column
		char* token = strtok(first_line, ",");

		// Iterate through comma separated values
		char name_string[10] = "name";
		for (int i = 0; token != NULL; i++) {
			// Check string equality
			if (strcmp(token, name_string) == 0) {
				num_col = i;
			}
			token = strtok(NULL, ",");
		}
	}
	return num_col;
}

