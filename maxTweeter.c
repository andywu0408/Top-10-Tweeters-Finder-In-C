#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_LINES_MAX 20000
#define LINE_CHARS_MAX 1024
#define TOP_NAMES 10

// main functions
void printTopTen(char* top_ten_names[], int top_ten_freqs[], int total_names);
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[], int total_names);
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[], int headerCount[]);
int getNameColumn (char* first_line, int headerCount[]);
void error();

// helper functions
int cmpfunc (const void * a, const void * b);
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[]);
void outerQuoteProcessor(char* str);

// gdb note: p final_freqs[x]@10

int main (int argc, char* argv[]) {
    char* tweetersName[FILE_LINES_MAX]; // for storing names of all tweeters
    int headerCount[1]; // for storing how many header fields we have
    headerCount[0] = 0;
    int total_names = 0; // for storing minimum amount of names to print

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
    int num_col = getNameColumn(first_line, headerCount);

    // Fill tweetersName array with all names, may have repeated names,
    int num_names = getTweetersName(file_ptr, num_col, tweetersName, headerCount); // total number of names
    
    // Store top 10
    char* top_ten_names[10]; // top 10 tweeters with most tweets
    int top_ten_freqs[10]; // frequencies of tweets made by top 10 tweeters
    
    // Check how many names to print (10 or less)
    if (num_names < TOP_NAMES) {
        total_names = num_names;
    } else {
        total_names = TOP_NAMES;
    }

    // Get names and frequencies of 10 different tweeters who made most tweets
    fillTopTenNamesAndFreq(tweetersName, num_names, top_ten_names, top_ten_freqs, total_names);
    
    // Print the names and frequencies of top 10 tweeters
    printTopTen(top_ten_names, top_ten_freqs, total_names);
    
    fclose(file_ptr);
}

void printTopTen(char* top_ten_names[], int top_ten_freqs[], int total_names) {
    for(int i = 0; i < total_names; i++){
        printf("%s: %d\n", top_ten_names[i], top_ten_freqs[i]);
    }
}

// Get names and frequencies of 10 different tweeters who made most tweets
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[], int total_names) {
    char* final_names[FILE_LINES_MAX]; // all names of tweeters, with no repeated element
    int final_freqs[FILE_LINES_MAX]; // frequencies of tweets made by all individual tweeters
    int num_final_elements = fillNamesAndFreq(names, num_names, final_names, final_freqs);
    
    // Sort frequencies in descending order
    qsort(final_freqs, num_final_elements, sizeof(int), cmpfunc);
    
    // fill in top ten or less
    for (int i = 0; i < total_names; i++) {
        top_ten_names[i] = final_names[i];
        top_ten_freqs[i] = final_freqs[i];
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
    int tweeter_name_length; 

    for(int i = 0; i < num_names; i++){
        if(strcmp(names[i], "") == 0){
            // skip because the element is repeated (we changed all repeated elements to empty string)
            continue; 
        } else {
            freq = 1; // initialize frequency to 1 
            current = names[i]; // store current name

            // check quoting of name
            outerQuoteProcessor(names[i]);
            
            for (int j = i + 1; j < num_names; j++) {

                // check quoting of compared name
                outerQuoteProcessor(names[j]);
                
                if (strcmp(names[j], "") == 0){
                    // skip because the element is repeated (we changed all repeated elements to empty string)
                    continue;
                }
                if (strcmp(current, names[j]) == 0) { // find another occurence of the element
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

/* outer quote validation helper function
    1. validate that we have same number of outer quotes (leading and trailing quotes)
    2. If match, strip off outermost quotes directly from names pointer
*/
void outerQuoteProcessor(char* str) {
    // store length of name
    int str_length = strlen(str);
    int leading_qm = 0;
    int trailing_qm = 0; //qm = quotation marks

    // count number of leading quotation marks
    for(int i = 0; i < str_length; i++){
        if(str[i] == '"'){ // skip
            leading_qm++;
        } else {
            break;
        }
    }
    // count number of trailing quotation marks
    for(int i = str_length-1; i >= 0; i--){
        if(str[i] == '"'){ // skip
            trailing_qm++;
        } else {
            break;
        }
    }
    
    if(leading_qm != trailing_qm){ // if leading & trailing quotation marks don't match, this is invalid field
        //printf("Error occured on (word: %s) with (leading qm: %d) and (trailing: %d)\n", str, leading_qm, trailing_qm);
        error();
    } else if(leading_qm == 0 & trailing_qm == 0){ // else if no outter quotation marks
        return; // do nothing
    }
    else { // else is valid field
        // copy unquoted portion over
        strncpy(str, str + 1, str_length - 2);
        // cut off oustanding quote with a null terminator
        str[str_length - 2] = '\0';
        //printf("str %s\n", str);
    }
}


/*
 Store names of all tweeters into tweetersName[] and return total number of names
 NOTE: The returned array may contain repeated names, or "empty" if no value given
*/
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[], int headerCount[]) {
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
        col_counter = 0;
        char* token = strsep(&temp, ",");
        // get name of tweeter from this line of file
        while(token != NULL){
            if(col_counter == num_col){ // if at the name column
                if(strcmp(token, "") == 0){ // name is "empty" if receive empty value
                    token = "empty";
                }
                tweetersName[counter] = token;
                //printf("token %s", token);
                //break;
            }
            token = strsep(&temp, ",");
            col_counter++;
        }
        counter++;

        // if more than 20,000 lines, then error
        if (counter > FILE_LINES_MAX - 1) {
            error();
        }

        // validate that we have same amount of content fields as header fields
        if (col_counter != headerCount[0]) {
            error();
        }
    }
    return counter;
}

// Get the column number for names
int getNameColumn (char* first_line, int headerCount[]) {
    
    int num_col = 0;
    int name_flag = 0;

    // Check if first line exists
    if (first_line == NULL) {
        error();
    }
    else {
        // Search first line for name column
        char* token = strtok(first_line, ",");

        // Looking for quoted "name" or unquoted name
        // In either Windows/Linux file format, since "name" could be the last column
        char name_string[10] = "name";
        char name_string_windows[10] = "name\r\n";
        char name_string_linux[10] = "name\n";

        // Iterate through comma separated values
        for (int i = 0; token != NULL; i++) {
            // quote validation & removal
            outerQuoteProcessor(token);

            // Check string equality to either variant of name
            if (strcmp(token, name_string) == 0 ||
                strcmp(token, name_string_windows) == 0 ||
                strcmp(token, name_string_linux) == 0) {
                num_col = i;
                name_flag = 1;
            }
            token = strtok(NULL, ",");
            headerCount[0]++; // increment header fields
        }
    }

    // if name was not found
    if (name_flag == 0) {
        error();
    } else { // otherwise return column position of name
        return num_col;
    }
    return num_col;
}

void error(){
    printf("Invalid Input Format");
    exit(1); // unsuccessful exit
}
