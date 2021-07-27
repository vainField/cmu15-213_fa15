#include "cachelab.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>

static int flag = 1;    // counter for LRU(least-recently used)

////////
// DT //
////////

typedef struct{
    int s, S, E, b, B;
    char* tf;
}Parameters;

typedef struct{
    char operation;
    int size;
    long address;
    int tag, set_index;
}Trace;

typedef struct{
    int valid;
    int tag, visit;
}CacheLine;

typedef struct{
    int hits, misses, evictions;
}Summary;

///////////////
// FUNCTIONS //
///////////////

void read_command(int argc, char** argv, Parameters* param){
    int opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                param->s = atoi(optarg);
                break;
            case 'E':
                param->E = atoi(optarg);
                break;
            case 'b':
                param->b = atoi(optarg);
                break;
            case 't':
                param->tf = optarg;
                break;
        }
    }
    param->S = 1 << param->s;
    param->B = 1 << param->b;
}

void init_cache(CacheLine** cache, Parameters param){
    for (int i=0; i<param.S; i++){
        cache[i] = (CacheLine *) malloc(sizeof(CacheLine) * param.E);
        for (int j=0; j<param.E; j++){
            cache[i][j].valid = 0, cache[i][j].tag = -1, cache[i][j].visit = 0;
        }
    }
}

void update_chache(CacheLine** cache, Parameters param, Summary* summary, Trace trace){
    CacheLine* set = cache[trace.set_index];
    if (trace.operation == 'M') summary->hits++;    // 'M': visit twice, at least one hit
    for (int i=0; i<param.E; i++){
        if (set[i].tag == trace.tag){
            set[i].visit = flag;
            summary->hits++;
            flag++;
            return ;
        }
    }
    int lru = INT32_MAX;
    int index;
    for (int i=0; i<param.E; i++){
        if (set[i].valid == 0){
            set[i].valid = 1, set[i].tag = trace.tag, set[i].visit = flag;
            summary->misses++;
            flag++;
            return ;
        }
        if (set[i].visit < lru) index = i, lru = set[i].visit;
    }
    set[index].tag = trace.tag, set[index].visit = flag;
    summary->misses++, summary->evictions++;
    flag++;
}

void free_cache(CacheLine** cache, Parameters param){
    for (int i=0; i<param.S; i++) free(cache[i]);
    free(cache);
}

//////////
// MAIN //
//////////

int main(int argc, char** argv){
    // get size of cache and tracefile from command line
    Parameters param;
    read_command(argc, argv, &param);
    //initialize cache
    CacheLine** cache = (CacheLine **) malloc(sizeof(CacheLine *) * param.S);
    init_cache(cache, param);
    //read trace file to update summary
    Trace trace;
    Summary summary;
    summary.evictions = 0, summary.hits = 0, summary.misses = 0;
    FILE* file = fopen(param.tf, "r");
    while(fscanf(file, " %c %lx,%d", &trace.operation, &trace.address, &trace.size) > 0){
        // skip 'I'
        if (trace.operation == 'I') continue;
        // find set_index and tag of trace
        trace.tag = trace.address >> (param.s + param.b);
        trace.set_index = (trace.address >> param.b) & (~(~0<<param.s));
        // find trace in cache
        update_chache(cache, param, &summary, trace);
    }
    // print summary
    printSummary(summary.hits, summary.misses, summary.evictions);
    // close and free
    fclose(file);
    free_cache(cache, param);
    return 0;
}
