#define _GNU_SOURCE
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

typedef struct{
    int lru_count;
    int valid;
    unsigned long long tag;
} block;

typedef struct{
    block *b_block; //sets consist of a series of blocks, based on associativity
} set;

int *convert_to_var(char **strings, int arg)
{
    int i = 1;
    int *var = (int *)malloc(sizeof(int)*4);
    var[3] = 0;
    // if the first argument is -h display usage info, else if it is -v
    // display trace info, else go through with the actual action.
    if(strcmp(strings[i], "-h") == 0) {
        //Print out Usage info here
        ++i;
    }
    
    if (strcmp(strings[i], "-v") == 0) {
        var[3] = 1;
        ++i;
    }
        var[0] = atoi(strings[i+1]);
        var[1] = atoi(strings[i+3]);
        var[2] = atoi(strings[i+5]);
    
    return var;
    //else go through with the action
}

set *cache_cons(int s, int E, int b){
    //function builts a cache
    int i, j;
    double set_no = pow(2,s); //total number of sets = 2^set bits
    set *s_set = (set *)malloc(sizeof(set)*set_no); //mallocs total number of sets in cache
    for (i = 0; i < set_no; i++){
        s_set[i].b_block = (block *)malloc(sizeof(block)*E);
        //mallocs for total number of blocks (lines) per set
        for (j = 0; j < E; j++){
            s_set[i].b_block[j].lru_count = 0; //sets values for each parameter in each block
            s_set[i].b_block[j].valid = 0;
            s_set[i].b_block[j].tag = 0;
        }
    }

    return s_set;
}

unsigned long long add_s(unsigned long long address, int b, int s){ //retrieves set index value from address
    address = address >> b;
    address = address << (64-s);
    address = address >> (64-s);
    return address;
}

unsigned long long add_tag(unsigned long long address, int b, int s){ //retrieves tag value from address
    int t = s + b;
    address = address >> t;
    return address;
}

void printCache(set *cache, int s, int E, int b)
{
    int set_no = pow(2,s);
    int i, j;
    for (i = 0; i < set_no; i++) {
        printf("Set %d:", i);
        for (j = 0; j < E; j++) {
            printf("%llx    ", cache[i].b_block[j].tag);
        }
        printf("\n");
    }
}

void sim(set* cache, unsigned long long address, int s, int E, int b, unsigned int *hits, unsigned int *misses, unsigned int *evictions, unsigned int v){
    unsigned long long tag_name, set_name;
    int counter = -1;
    int val = -1;
    set_name = add_s(address, b, s); //set number retrieved from memory address
    tag_name = add_tag(address, b, s); //tag number retrieved from memory address

    for (int i = 0; i < E; i++) {
        //printf("TAG: %llx\n", tag_name);
        if (cache[set_name].b_block[i].tag == tag_name && cache[set_name].b_block[i].valid == 1){
            ++(*hits); //if set numbers and tags match + bit is valid => hit
            if(v) {
                printf("hit ");
            }
            for (int j = 0; j < E; j++) {
                if (j != i) {
                    ++cache[set_name].b_block[j].lru_count;
                } else {
                    cache[set_name].b_block[j].lru_count = 0;
                }
            }
            break;
            //++cache[set_name].b_block[i].lru_count;
        } else if (cache[set_name].b_block[i].valid == 0){
            ++(*misses); //if bit for address is not valid then miss
            if(v) {
                printf("miss ");
            }
            cache[set_name].b_block[i].valid = 1;
            cache[set_name].b_block[i].tag = tag_name;
            for (int j = 0; j < E; j++) {
                if (j != i) {
                    ++cache[set_name].b_block[j].lru_count;
                } else {
                    cache[set_name].b_block[j].lru_count = 0;
                }
            }
            break;
            //++cache[set_name].b_block[i].lru_count;
        } else if (cache[set_name].b_block[i].valid == 1 && i+1 == E) {
            ++(*misses);
            if(v) {
                printf("miss ");
            }
            for (int j = 0; j < E; j++) {
                if (cache[set_name].b_block[j].lru_count > val) {
                    val = cache[set_name].b_block[j].lru_count;
                    counter = j;
                }
            }
            for (int j = 0; j < E; j++) {
                if (j != counter) {
                    ++cache[set_name].b_block[j].lru_count;
                } else {
                    cache[set_name].b_block[j].lru_count = 0;
                }
            }
            ++(*evictions);
            if(v) {
                printf("eviction ");
            }
            cache[set_name].b_block[counter].tag = tag_name;
        }
    }

}

void retrieves_line(int s, int E, int b, FILE *f, unsigned int *hits, unsigned int *misses, unsigned int *evictions, unsigned int v){
    unsigned int i, intvalue;
    size_t len = 400;
    char *buffer = (char *)malloc(sizeof(char)*len);
    unsigned long long address = 0;
    set* c = cache_cons(s, E, b); //new cache is stored in c

    while(fgets(buffer,len,f)){ // reads each line from the file
        if (buffer[0] == ' '){ //only applicable to parse if first character is [space]
            char op = buffer[1]; //operation is the next character
            buffer[strlen(buffer)-1] = ' ';
            if (v)
                printf("%s", buffer);
            for (i = 3; buffer[i] != ','; i++){
                address *= 0x10;
                if (buffer[i] >= 'a')
                    intvalue = buffer[i] - 'a' + 10;
                else
                    intvalue = buffer[i] - '0';
                address += intvalue; //address is copied from a char* to a long long
            }
            if (op == 'M') {
                sim(c, address, s, E, b, hits, misses, evictions, v); //simulates twice if modify operation
            }
            sim(c, address, s, E, b, hits, misses, evictions, v); //simulates cache
            address = 0;
        }
        if(v) {
            printf("\n");
        }
    }
    
    printCache(c, s, E, b);
    
    free(c);
}

int main(int argc, char *argv[])
{
    unsigned int s = 0, E = 0, b = 0, hits = 0, misses = 0, evictions = 0, v;
    FILE *f = fopen(argv[argc-1], "r");
    if (f == NULL) {
        fprintf(stderr, "No File\n");
        exit(1);
    }
    int *variables = (int *)malloc(sizeof(int)*4);
    variables = convert_to_var(argv, argc);
    s = variables[0];
    E = variables[1];
    b = variables[2];
    v = variables[3];
    
    //printf("%d\n", v);
    
    retrieves_line(s, E, b, f, &hits, &misses, &evictions,v);
    printSummary(hits, misses, evictions);
    return 0;
}
