#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_LINES_MAX 20000
#define LINE_CHARS_MAX 1024

void error();
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[]);
int getNameColumn (char* first_line);
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[]);
void printTopTen(char* top_ten_names[], int top_ten_freqs[]);

// helper functions
int cmpfunc (const void * a, const void * b);
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[]);

// gdb note: p *final_freqs@100

int main (int argc, char* argv[]) {
    char* tweetersName[FILE_LINES_MAX]; // for storing name of all tweeters
    
	// Check for valid program input
	// Must have 1 command line argument (filename)
	if (argc != 2) {
		error();
	}

	// Store and open file
	const char* file_name = argv[1];
	FILE* file_ptr = fopen(file_name, "r");

	// Check if file exists
	if (file_ptr == NULL) {
		error();
	}

	// Store first line 
	char first_line[LINE_CHARS_MAX + 2]; // "+2" for potential overflow char + null term char
    if (fgets(first_line, LINE_CHARS_MAX + 2, file_ptr) == NULL) { // fgets includes null-terminated char (strlen does not)
        error(); 
    } else if (strlen(first_line) > LINE_CHARS_MAX) { 
        // if we read in more than 1024 chars
        error();
    }

	// Get column position of "name" 
	int num_col = getNameColumn(first_line);

    // Fill tweetersName array with all names, may have repeated names,
    int num_names = getTweetersName(file_ptr, num_col, tweetersName); // total number of names
    
    // Store top 10
    char* top_ten_names[10]; // top 10 tweeters with most tweets
    int top_ten_freqs[10]; // frequencies of tweets made by top 10 tweeters
    
    // Get names and frequencies of 10 different tweeters who made most tweets
    fillTopTenNamesAndFreq(tweetersName, num_names, top_ten_names, top_ten_freqs);
    
    // Print the names and frequencies of top 10 tweeters
    printTopTen(top_ten_names, top_ten_freqs);
    
    fclose(file_ptr);
}

void printTopTen(char* top_ten_names[], int top_ten_freqs[]){
    for(int i = 0; i < 10; i++){
        printf("%s: %d\n", top_ten_names[i], top_ten_freqs[i]);
    }
}

// Get names and frequencies of 10 different tweeters who made most tweets
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[]){
    char* final_names[FILE_LINES_MAX]; // all names of tweeters, with no repeated element
    int final_freqs[FILE_LINES_MAX]; // frequencies of tweets made by all individual tweeters
    int num_final_elements = fillNamesAndFreq(names, num_names, final_names, final_freqs);
    
    // Sort frequencies in descending order
    qsort(final_freqs, num_final_elements, sizeof(int), cmpfunc);
    
    int counter = 0;
    
    for(int i = 0; i < 10; i++) { // get top ten highest
        top_ten_names[counter] = final_names[i];
        top_ten_freqs[counter] = final_freqs[i];
        counter++;
    };
}
// Comparable function for qsort
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)b - *(int*)a );
}

// helper function for fillTopTenNamesAndFreq function
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[]){
    int freq; // counter
    char* current;
    int num_final_elements = 0; // unique names

    for(int i = 0; i < num_names; i++){
        if(strcmp(names[i], "") == 0){
            // skip because the element is repeated (we changed all repeated elements to empty string)
            continue; 
        } else {
            freq = 0;
            current = names[i];
            freq++;
            for(int j = i + 1; j < num_names; j++){
                if(strcmp(names[j], "") == 0){
                    // skip because the element is repeated (we changed all repeated elements to empty string)
                    continue;
                }
                if(strcmp(current, names[j]) == 0){ // find another occurence of the element
                    freq++;
                    // change it to empty string, so we can handle this repeated value when we encounter it later on
                    strcpy(names[j], "");
                }
            }
            final_names[num_final_elements] = current;
            final_freqs[num_final_elements] = freq;
            num_final_elements++;
        }
    }
    
    if (num_final_elements <= 0) {
        error();
    }
    
    return num_final_elements;
}


/*
 Store names of all tweeters into tweetersName[] and return total number of names
 NOTE: The returned array may contain repeated names, or "empty" if no value given
*/
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[]) {
    char line[LINE_CHARS_MAX + 2]; // one line in file
    char* temp; // store pointer to line
    int col_counter;
    int counter = 0;
    
    // get one line from file if not end of file yet
    while(fgets(line, LINE_CHARS_MAX + 2, file_ptr ) != NULL ) {
        if (strlen(line) > LINE_CHARS_MAX) { 
            // if we read in more than 1024 chars
            error();
        }

        temp = strdup(line);
        col_counter = 1;
        char* token = strsep(&temp, ",");
        // get name of tweeter from this line of file
        while(token != NULL){
            if(col_counter == num_col + 1){ // if at the name column
                if(strcmp(token, "") == 0){ // name is "empty" if receive empty value
                    token = "empty";
                }
                tweetersName[counter] = token;
                break;
            }
            token = strsep(&temp, ",");
            col_counter++;
        }
        counter++;
    }
    return counter;
}

void error(){
    printf("Invalid Input Format");
    exit(1); // unsuccessful exit
}

// Get the column number for names
int getNameColumn (char* first_line) {
	
	int num_col = 0;
	// Check if first line exists
	if (first_line == NULL) {
		error();
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

