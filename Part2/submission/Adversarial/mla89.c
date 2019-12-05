#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define _GNU_SOURCE
#include <string.h>

int field_count;
char** fields;
int name_index;

// The linked node to store infos
struct count_node
{
    char* name;
    int number;
    struct count_node *next;
};

// Globle head of nodes
struct count_node *head;

// invalid exit
void exit_invalid(FILE* stream){
    if(stream != NULL){
        fclose(stream);
    }

    fprintf(stderr, "Invalid input format.\n");
    exit(1);
}


// Create a global array for storing the headers.
void fields_creation(char* header_line){
    int comma_count = 0;
    for(int i = 0; i < strlen(header_line); i ++){
        if(header_line[i] == ','){
            comma_count ++;
        }
    }
    
    field_count = comma_count + 1;
    fields = malloc(field_count * sizeof(char*));
    int index = 0;
    int prev = 0;
    for(int i = 0; i < strlen(header_line); i ++){
        if(header_line[i] == ','){
            if(prev == i){
                fields[index] = "";
                index ++;
                prev = i + 1;
            }
            else{
                fields[index] = (char*)malloc(i - prev + 1);
                memcpy(fields[index], &header_line[prev], i - prev);
                fields[index][i - prev] = '\0';
                index ++;
                prev = i + 1;
            }
        }
    }

    // The last field
    if(strlen(header_line) == prev){
        fields[index] = "";
    }
    else{
        fields[index] = malloc(strlen(header_line) - prev + 1);
        strncpy(fields[index], &header_line[prev], strlen(header_line) - prev);
        fields[index][strlen(header_line) - prev] = '\0';
    }
}


// Add the tweeter into the node or increment if exists.
void increment_node(char* name){
    struct count_node *trav_node = head->next;
    while(trav_node != NULL){
        if(strcmp(trav_node->name, name) == 0){
            trav_node->number ++;
            return;
        }
        trav_node = trav_node->next;
    }
    struct count_node *new_node = malloc(sizeof (struct count_node));

    if(strcmp(name, "") != 0){
        new_node->name = malloc(strlen(name) + 1);
        strcpy(new_node->name, name);
        new_node->name[strlen(name)] = '\0';
    }
    else{
        new_node->name = "";
    }
    new_node->number = 1;
    new_node->next = NULL;
    
    if(head->next == NULL){
        head->next = new_node;
    }
    else{
        struct count_node *second_node = head->next;
        head->next = new_node;
        new_node->next = second_node;
    }
}

// validate the content and store the name and tweets count
bool content_validator_collector(char* line){
    int comma_count = 0;
    for(int i = 0; i < strlen(line); i ++){
        if(line[i] == ','){
            comma_count ++;
        }
    }

    int item_count = comma_count + 1;
    if(item_count != field_count){
        return false;
    }
    char** items = malloc(item_count * sizeof(char*));
    int index = 0;
    int prev = 0;
    for(int i = 0; i < strlen(line); i ++){
        if(line[i] == ','){
            if(prev == i){
                items[index] = "";
                index ++;
                prev = i + 1;
            }
            else{
                items[index] = malloc(i - prev + 1);
                memcpy(items[index], &line[prev], i - prev + 1);
                items[index][i - prev] = '\0';
                index ++;
                prev = i + 1;
            }
        }
    }

    // The last field
    if(strlen(line) == prev){
        items[index] = "";
    }
    else{
        items[index] = malloc(strlen(line) - prev + 1);
        memcpy(items[index], &line[prev], strlen(line) - prev + 1);
        items[index][strlen(line) - prev] = '\0';
    }

    for(int i = 0; i < field_count; i ++){
        int length = strlen(items[i]);
        if(strcmp(items[i], "\"") == 0){
            return false;
        }
        if(strcmp(items[i], "") != 0){
            if (items[i][length - 1] == '\"' ^ items[i][0] == '\"'){
                return false;
            }
        }

        if(strcmp(fields[i], "") == 0 || fields[i][0] != '\"'){
            if(!(items[i][0] != '\"')){
                return false;
            }
        }
        else{
            if(!(items[i][0] == '\"')){
                return false;
            }
        }
    }

    increment_node(items[name_index]);

    // Free the memory
    for(int i = 0; i < field_count; i ++){
        if(strcmp(items[i], "") != 0){
            free(items[i]);
        }
    }

    free(items);
    return true;
}

// Validate the fields.
bool fields_validator(){
    int name_count = 0;
    for(int i = 0; i < field_count; i ++){
        int length = strlen(fields[i]);

        if(strcmp(fields[i], "") != 0){
            if (fields[i][length - 1] == '\"' ^ fields[i][0] == '\"'){
                return false;
            }
        }

        if(strcmp(fields[i], "\"") == 0){
            return false;
        }

        if(strcmp(fields[i], "name") == 0 || strcmp(fields[i], "\"name\"") == 0){
            name_count ++;
            // Get the name index
            name_index = i;
        }
    }

    if(name_count != 1){
        return false;
    }

    return true;
}

int comparator(const void * num1, const void * num2) 
{
    int n1 = *((int*)num1);
    int n2 = *((int*)num2);
    return -(n1 - n2);
}

// Print the final result of max tweeters.
void print_top_ten(){
    if(head->next == NULL){
        return;
    }

    struct count_node *trav_node = head->next;
    int count = 0;
    while(trav_node != NULL){
        count ++;
        trav_node = trav_node->next;
    }

    int* times_array = malloc(count * sizeof(int));

    trav_node = head->next;
    int index = 0;
    while(trav_node != NULL){
        times_array[index] = trav_node->number;
        index ++;
        trav_node = trav_node->next;
    }

    qsort(times_array, count, sizeof(int), comparator);
    int max_index = 0;

    for(int i = 0; i < 10; i ++){
        struct count_node *prev_node = head;
        trav_node = head->next;
        while(trav_node != NULL){
            if(trav_node->number == times_array[max_index]){
                if(strcmp(trav_node->name, "") != 0 && trav_node->name[0] == '\"'){
                    char* remove_quotes = (char*)malloc(strlen(trav_node->name) - 1);
                    memcpy(remove_quotes, (trav_node->name) + 1, strlen(trav_node->name) - 2);
                    remove_quotes[strlen(trav_node->name) - 2] = '\0';
                    printf("%s: %d\n", remove_quotes, trav_node->number);
                    free(remove_quotes);
                }
                else{
                    printf("%s: %d\n", trav_node->name, trav_node->number);
                }
                max_index ++;
                prev_node->next = trav_node->next;
                if(strcmp(trav_node->name, "") != 0){
                    free(trav_node->name);
                }
                free(trav_node);
                trav_node = prev_node->next;
                break;
            }

            prev_node = trav_node;
            trav_node = trav_node->next;
        }
    }


    trav_node = head->next;
    while(trav_node != NULL){
        struct count_node *temp = trav_node;
        trav_node = trav_node->next;
        if(strcmp(temp->name, "") != 0){
            free(temp->name);
        }
        free(temp);
    }

    free(head);
    free(times_array);
}

int main(int argc, char *argv[]) {
    // Only one arg is accepted.
    if(argc != 2){
        fprintf(stderr, "Please only provide one argument.\n");
        exit(1);
    }

    // Open the file with fopen().
    FILE *file_stream;
    file_stream = fopen(argv[1], "r");
    ssize_t read_bytes;
    size_t len = 0;
    char *line = NULL;

    // Check if the file exist or not.
    if(file_stream == NULL){
        exit_invalid(file_stream);
    }

    // Go to the end of file.
    fseek(file_stream, 0, SEEK_END);
    // Get the postion of offset which is the same as file size.
    if(ftell(file_stream) == 0 || ftell(file_stream) > 20000 * 1024){
        exit_invalid(file_stream);
    }

    // Restore the offset to the begining.
    fseek(file_stream, 0, SEEK_SET);

    // Take the header and check if it is valid.
    if((read_bytes = getline(&line, &len, file_stream)) != -1){
        if(read_bytes > 1024){
            exit_invalid(file_stream);
        }

        char* temp = malloc(strlen(line) + 1);
        strcpy(temp, line);
        temp[strlen(line)] = '\0';
        strtok(temp, "\r\n");
        strtok(NULL, "\n");

        // Empty header, exit.
        if(strcmp(line, "") == 0){
            exit_invalid(file_stream);
        }

        fields_creation(temp);
        free(temp);
    }
    else{
        exit_invalid(file_stream);
    }

    if(!fields_validator()){
        exit_invalid(file_stream);
    }

    head = malloc(sizeof(struct count_node));
    head->next = NULL;
    head->number = -1;
    free(line);
    line = NULL;

    // Go through the left lines of the file, check and collect.
    int line_number = 1;
    while((read_bytes = getline(&line, &len, file_stream)) != -1){
        if(line_number > 20000){
            exit_invalid(file_stream);
        }

        if(read_bytes > 1024){
            exit_invalid(file_stream);
        }

        char* temp = malloc(strlen(line) + 1);
        strcpy(temp, line);
        temp[strlen(line)] = '\0';
        strtok(temp, "\r\n");
        strtok(NULL, "\n");

        if(strcmp(temp, "") == 0){
            exit_invalid(file_stream);
        }

        if(!content_validator_collector(line)){
            exit_invalid(file_stream);
        }
        line_number ++;
        free(temp);
        free(line);
        line = NULL;
    }

    for(int i = 0; i < field_count; i ++){
        if(strcmp(fields[i], "") != 0){
            free(fields[i]);
        }
    }

    // free memory.
    free(line);
    free(fields);
    print_top_ten();
    fclose(file_stream);
}