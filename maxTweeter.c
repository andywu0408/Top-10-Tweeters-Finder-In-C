#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define FILE_LINES_MAX 20000
#define LINE_CHARS_MAX 1024
#define TOP_NAMES 10

// csv information tracking
struct csvInformation {
    bool quoted_header; // for storing whether the header is quoted
    bool newline_quoted_flag; // for storing whether the header is quoted AND has a newline
    bool windows_flag; // for storing whether the header has a Windows style newline (quoted)
    bool linux_flag; // for storing whether the header has a Linux style newline (quoted)
    int header_count; // for storing how many header fields we have
    int total_names; // for storing minimum amount of names to print
};

// main functions
void printTopTen(char* top_ten_names[], int top_ten_freqs[], struct csvInformation* csvInfo);
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[], struct csvInformation* csvInfo);
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[], struct csvInformation* csvInfo);
int getNameColumn (char* first_line, struct csvInformation* csvInfo);

// helper functions
int cmpfunc(const void * a, const void * b);
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[], struct csvInformation* csvInfo);
void outerQuoteProcessor(char* str, struct csvInformation* csvInfo);
void error();

// gdb note: p final_freqs[x]@10

int main (int argc, char* argv[]) {

    // declare & initialize struct
    struct csvInformation csvInfo = {false, 0, 0};

    // for storing names of all tweeters
    char* tweetersName[FILE_LINES_MAX]; 

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
        printf(" ");
        exit(0); // if empty csv, print nothing
    } else if (strlen(first_line) > LINE_CHARS_MAX) { 
        // if we read in more than 1024 chars
        error();
    }

    // Get column position of "name" 
    int num_col = getNameColumn(first_line, &csvInfo);

    // Fill tweetersName array with all names, may have repeated names,
    int num_names = getTweetersName(file_ptr, num_col, tweetersName, &csvInfo); // total number of names
    
    // Declare top 10 tweeter/frequency array structures
    char* top_ten_names[10]; // top 10 tweeters with most tweets
    int top_ten_freqs[10]; // frequencies of tweets made by top 10 tweeters

    // Get names and frequencies of 10 different tweeters who made most tweets
    fillTopTenNamesAndFreq(tweetersName, num_names, top_ten_names, top_ten_freqs, &csvInfo);
    
    // Print the names and frequencies of top 10 tweeters
    printTopTen(top_ten_names, top_ten_freqs, &csvInfo);
    
    fclose(file_ptr);
}

void printTopTen(char* top_ten_names[], int top_ten_freqs[], struct csvInformation* csvInfo) {
    
    // if name header is quoted, then remove quotes before printing
    if (csvInfo->quoted_header == true) {
        // for names to be printed
        for (int i = 0; i < csvInfo->total_names; i++) {
            int tweeter_name_length = strlen(top_ten_names[i]); // length of name

            // if windows new line, we must remove 2 from length of string
            if (csvInfo->windows_flag) {
                tweeter_name_length = tweeter_name_length - 2;
            // if linux new line, we must remove 1 from length of string
            } else if (csvInfo->linux_flag) {
                tweeter_name_length = tweeter_name_length - 1;
            }

            strncpy(top_ten_names[i], top_ten_names[i] + 1, tweeter_name_length); // copy unquoted portion over
            top_ten_names[i][ tweeter_name_length - 2 ] = '\0'; // cut off remaining quoted part with null terminator
            printf("%s: %d\n", top_ten_names[i], top_ten_freqs[i]); // print
        }   
    } else { // otherwise, print normally
        for (int i = 0; i < csvInfo->total_names; i++){
            printf("%s: %d\n", top_ten_names[i], top_ten_freqs[i]);
        }
    }
}

// Get names and frequencies of 10 different tweeters who made most tweets
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[], struct csvInformation* csvInfo) {
    char* final_names[FILE_LINES_MAX]; // all names of tweeters, with no repeated element
    int final_freqs[FILE_LINES_MAX]; // frequencies of tweets made by all individual tweeters
    
    // Initialize frequencies to 0
    for (int i = 0; i < 10; i++) {
        final_freqs[i] = 0;
    }

    int num_final_elements = fillNamesAndFreq(names, num_names, final_names, final_freqs, csvInfo);
    
    // Sort frequencies in descending order
    qsort(final_freqs, num_final_elements, sizeof(int), cmpfunc);
    
    // Check how many names to print (10 or less)
    // 1. Do we have at least 10 tweeter names
    int names_to_print = 0;
    for (int i = 0; i < 10; i++) {
        // if frequency is not 0, then increment
        if (final_freqs[i] != 0) { 
            names_to_print++;
        }
    } // 2. If we have less than 10 names, then make sure we print only that many
    if (names_to_print < TOP_NAMES) {
        csvInfo->total_names = names_to_print;
    } else {
        csvInfo->total_names = TOP_NAMES;
    }

    // fill in top ten or less
    for (int i = 0; i < csvInfo->total_names; i++) {
        top_ten_names[i] = final_names[i];
        top_ten_freqs[i] = final_freqs[i];
    };
}

// Comparable function for qsort
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)b - *(int*)a );
}

// helper function for fillTopTenNamesAndFreq function
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[], struct csvInformation* csvInfo) {
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

            // check quoting of current name
            outerQuoteProcessor(names[i], csvInfo);
            
            for (int j = i + 1; j < num_names; j++) {

                // if name is empty string
                if (strcmp(names[j], "") == 0) {
                    continue; // then skip (we changed all repeated elements to empty string)
                }

                // check quoting of compared (next) name
                outerQuoteProcessor(names[j], csvInfo);
                
                // if name is repeated
                if (strcmp(current, names[j]) == 0) { 
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
void outerQuoteProcessor(char* str, struct csvInformation* csvInfo) {
    // store length of name
    int str_length = strlen(str);
    int leading_qm = 0;
    int trailing_qm = 0; //qm = quotation marks

    // for adapting for loop array bounds for quote checking
    // windows newlines will add 2 extra chars "\r\n"
    // linux newlines will add 1 extra char "\n"
    if (csvInfo->windows_flag == true) { // if windows newline (quoted)
        str_length = str_length - 2;
    } else if (csvInfo->linux_flag == true) { // if linux newline (quoted)
        str_length = str_length - 1;
    }

    // if str length is less than 2 AND header was quoted, then error
    if (str_length < 2 && csvInfo->quoted_header == true) {
        error();
    }

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
    
    if (leading_qm != trailing_qm) { // if quotes mismatch, then error
        //printf("Error occured on (word: %s) with (leading qm: %d) and (trailing: %d)\n", str, leading_qm, trailing_qm);
        error();
    } else if (leading_qm == 0 & trailing_qm == 0) { // else if no outer quotes
        
        // if name header is not quoted, then error
        if (csvInfo->quoted_header == true) {
            error();
        }
    }
    else { // else quotes match

        // if name header is quoted, then error
        if (csvInfo->quoted_header == false) {
            error();
        }
    }
}


/*
 Store names of all tweeters into tweetersName[] and return total number of names
 NOTE: The returned array may contain repeated names, or "empty" if no value given
*/
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[], struct csvInformation* csvInfo) {
    char line[LINE_CHARS_MAX + 2]; // one line in file
    char* temp; // store pointer to line
    int col_counter = 0;
    int counter = 0;
    
    // get one line from file if not end of file yet
    while(fgets(line, LINE_CHARS_MAX + 2, file_ptr ) != NULL ) {
        if (strlen(line) > LINE_CHARS_MAX) { 
            // if we read in more than 1024 chars
            error();
        }

        temp = strdup(line); // copy line
        col_counter = 0; // counting columns for valid csv
        char* token = strsep(&temp, ","); // first token
        // get name of tweeter from this line of file
        while (token != NULL) {
            if (col_counter == num_col) { // if at the name column
                if(strcmp(token, "") == 0){ // name is "empty" if receive empty value
                    token = "";
                }
                tweetersName[counter] = token;
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
        if (col_counter != csvInfo->header_count) {
            error();
        }
    }

    return counter;
}

// Get the column number for names
int getNameColumn (char* first_line, struct csvInformation* csvInfo) {
    char* temp; // store pointer to line
    int num_col = 0;
    int name_flag = 0;

    // Check if first line exists
    if (first_line == NULL) {
        error();
    }
    else { // Search first line for name column
        temp = strdup(first_line); // copy line
        char* token = strsep(&temp, ","); // first token

        // Looking for quoted "name" or unquoted name
        // In either Windows/Linux file format, since "name" could be the last column
        char name_string[10] = "name";
        char name_string_windows[10] = "name\r\n";
        char name_string_linux[10] = "name\n";
        char name_string_quoted[10] = "\"name\"";
        char name_string_quoted_windows[10] = "\"name\"\r\n";
        char name_string_quoted_linux[10] = "\"name\"\n";

        // Iterate through comma separated fields
        for (int i = 0; token != NULL; i++) {

            // Check string equality to either variant of name
            // Unquoted name with or without newline (same treatment)
            if (strcmp(token, name_string) == 0 ||
                strcmp(token, name_string_windows) == 0 ||
                strcmp(token, name_string_linux) == 0) {
                num_col = i;
                name_flag = 1;
            // Quoted "name" 
            } else if (strcmp(token, name_string_quoted) == 0) {
                num_col = i;
                name_flag = 1;
                csvInfo->quoted_header = true;
            // Quoted "name" with Windows newline
            } else if (strcmp(token, name_string_quoted_windows) == 0) {
                num_col = i;
                name_flag = 1;
                csvInfo->newline_quoted_flag = true;
                csvInfo->windows_flag = true;
                csvInfo->quoted_header = true;
            // Quoted "name" with Linux newline
            } else if (strcmp(token, name_string_quoted_linux) == 0) {
                num_col = i;
                name_flag = 1;
                csvInfo->newline_quoted_flag = true;
                csvInfo->linux_flag = true;
                csvInfo->quoted_header = true;
            }
            token = strsep(&temp, ","); // read next field
            csvInfo->header_count++; // increment header fields
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
