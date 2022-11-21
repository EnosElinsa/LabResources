#define _GNU_SOURCE
#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FILENAME_LENGTH 100

static int s; // Number of set index bits(S = 2 ^ s is the number of sets)
static int E; // Associativity(number of lines per set)
static int b; // Number of block bits(B = 2 ^ b is the block size)
static int S; // Number of sets
static int hit_count = 0;
static int miss_count = 0;
static int eviction_count = 0;
static char* trace_file_directory; // valgrind trace file directory

// A double linkedList node to simulate a line in a set
typedef struct Line {
    unsigned tag;       // Tag bits
    unsigned set_index; // Set index bits(actually it makes no difference)
    struct Line* prev;  // Pointer to the previous line
    struct Line* next;  // Pointer to the next line
} Line;

// A double linkedList to simulate and arrange a set in the cache
typedef struct LRU {
    Line* head;   // Dummy Head pointer of a LRU set
    Line* tail;   // Dummy Tail pointer of a LRU set
    int size;     // The size of s LRU set(associativity / number of lines per set)
} LRU;

static LRU* cache; // The whole cache

void parseCommandLineArguments(int argc, char** argv);
void intializeCache();
void insertAtHead(LRU* current_LRU, Line* new_line);
void delete(LRU* current_LRU, Line* line_to_be_deleted);
Line* findByTag(LRU* current_LRU, unsigned target_tag);
void update(unsigned address);
void simulate();

int main(int argc, char** argv)
{
    parseCommandLineArguments(argc, argv);
    intializeCache();
    simulate();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

/**
 * @brief Parsing command line arguments to get s(number of set index bits), E(number of lines per set), 
 * b(number of blocks bits) and trace file directory via getopt function.
 * @param argc argument count
 * @param argv argument vector(parameter list)
 */
void parseCommandLineArguments(int argc, char** argv) {
    int option;
    trace_file_directory = (char*)malloc(MAX_FILENAME_LENGTH * sizeof(char));
    while ((option = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch(option) {
        case 's': s = atoi(optarg);
        case 'E': E = atoi(optarg);
        case 'b': b = atoi(optarg);
        case 't': strcpy(trace_file_directory, optarg);
        }
    }
    S = 1 << s;
}

/**
 * @brief Set each set in the cache an empty double linked list.
 */
void intializeCache() {
    cache = (LRU*)malloc(S * sizeof(LRU));
    for (int set_index = 0; set_index < S; set_index++) {
        LRU* current_LRU = &cache[set_index];
        // Assign memory
        current_LRU->head = (Line*)malloc(sizeof(Line));
        current_LRU->tail = (Line*)malloc(sizeof(Line));

        current_LRU->head->next = current_LRU->tail;
        current_LRU->tail->prev = current_LRU->head;

        current_LRU->head->prev = NULL;
        current_LRU->tail->next = NULL;

        cache[set_index].size = 0;
    }
}

/**
 * @brief Insert a specific line to a LRU set.
 * @param current_LRU The LRU set to insert a line
 * @param new_line  The line to be inserted
 */
void insertAtHead(LRU* current_LRU, Line* new_line) {
    new_line->next = current_LRU->head->next;
    new_line->prev = current_LRU->head;
    current_LRU->head->next->prev = new_line;
    current_LRU->head->next       = new_line;
    current_LRU->size++;
}

/**
 * @brief Delete(evict) a specific line in a LRU set.
 * @param current_LRU  The LRU set to be manipulated
 * @param line_to_be_deleted  The line to to deleted
 */
void delete(LRU* current_LRU, Line* line_to_be_deleted) {
    line_to_be_deleted->prev->next = line_to_be_deleted->next;
    line_to_be_deleted->next->prev = line_to_be_deleted->prev;
    current_LRU->size--;
}

/**
 * @brief Find the indicated line in a LRU set via the tag bit.
 * @param current_LRU The LRU line to perform the search
 * @param target_tag  The tag bits of the line to be found
 * @return Line* NULL if the line cannot be found
 */
Line* findByTag(LRU* current_LRU, unsigned target_tag) {
    Line* current_line = current_LRU->head->next;
    while (current_line != current_LRU->tail) {
        if (current_line->tag == target_tag) {
            return current_line;
        }
        current_line = current_line->next;
    }
    return NULL;
}

/**
 * @brief The core process of the cache simulator.
 * 1. Get the tag bits and the set index bits from the address.
 * (The block bias bits can be ingnored here in that it makes no difference in the simulating process)
 * 2. Get the reference of the indicated LRU set via the set index bits and find the specific line via
 * the tag bits.
 * 3. If the indicated line can be found in the specific LRU set which means a hit from the cache, then
 * increments the hit_count and insert this line to the headmost position of its LRU set according the
 * LRU strategy.
 * 4. If the indicated line cannot be found in the specific LRU set which means a miss from the cache,
 * then increments the miss_count. 
 *    Next, create a new line with the address(this simulates the process of loading a block from the 
 * memory) and insert it to the headmost position of its LRU set if the set is not full. 
 *    If the set is full which means we need to evict a block(increment the eviction_count), we remove 
 * the backmost line of the set which indicates the least frequently used line according to the LRU 
 * strategy and insert the new line to the headmost position.
 * @param address The address of a trace of its memory access
 */
void update(unsigned address) {
    unsigned tag = address >> (s + b);
    unsigned set_index = (address >> b) & (0xFFFFFFFF >> (32 - s));
    LRU* current_LRU = &cache[set_index];
    Line* target_line = findByTag(current_LRU, tag);
    if (target_line != NULL) {
        // hit
        hit_count++;
        // If the target line is already the headmost line in the set, it do not need to be moved.
        if (current_LRU->head->next != target_line) {
            delete(current_LRU, target_line);
            insertAtHead(current_LRU, target_line);
        }
    }
    else {
        // miss
        miss_count++;

        Line* new_line = (Line*)malloc(sizeof(Line));
        new_line->next = NULL;
        new_line->prev = NULL;
        new_line->tag = tag;
        new_line->set_index = set_index;

        if (current_LRU->size == E) {
            // evict
            delete(current_LRU, current_LRU->tail->prev);
            eviction_count++;
        }
        insertAtHead(current_LRU, new_line);
    }
}

/**
 * @brief Simulation process.
 */
void simulate() {
    FILE* fp = fopen(trace_file_directory, "r");
    char operation;
    unsigned address;
    int size;
    while(fscanf(fp, "%c %x, %d", &operation, &address, &size) > 0) {
        switch (operation)
        {
        case 'L': update(address); break;
        case 'M': update(address);
        case 'S': update(address); break;
        }
    }
}