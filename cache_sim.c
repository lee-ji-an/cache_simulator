#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define FALSE 0
#define TRUE 1

typedef struct cacheMemory
{
    int valid, tag, * data, dirty, seq;
}cacheMemory;

typedef struct mainMemory {
    unsigned long* data;
    unsigned long tag;
    unsigned long idx;
}mainMemory;

void init();
void read_data(FILE* fp);
int logTwo(int number);
void find_cache(int cal_tag);
void find_mem(int idx, int tag);
void save_cache(int idx, int i, int word);
void load_mem(int idx, int j);
int find_seq(int idx);

int cache_size = 64;
int block_size = 8;
int set_size = 2;
int word_cnt;
char trace[100] = "sample.txt";
int cache_line;
int set_num;
int seq;
int set_two;
int word_two;
int block_two;
int i, min_seq = 999999, min_idx;
int cal_tag;
int mem_idx;
int mem_exist;
int cache_exist;
int* mem_data;
int j, k;
int dirty_cnt;
float cycle_cnt;
float miss_rate;
int mem_size = 30;
// int cycle_cnt;

FILE* fp = 0;

unsigned long address;
char mode;
long data;
int idx;
int word;

int hit_cnt;
int miss_cnt;

cacheMemory** myCache;
mainMemory* myMemory;

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
       if (argv[i][0] == '-')
       {
           if (argv[i][1] == 's')
               cache_size = atoi(argv[i] + 3);

           if (argv[i][1] == 'b')
               block_size = atoi(argv[i] + 3);

           if (argv[i][1] == 'a')
               set_size = atoi(argv[i] + 3);
       }
    }

    // initialize data
    init();


    fp = fopen(trace, "r");

    if (fp == NULL)
    {
        printf("[error] error opening file");
        fflush(stdout);
        exit(-1);
    }

    set_two = logTwo(set_num);
    word_two = logTwo(word_cnt);
    block_two = logTwo(block_size);

    while (!feof(fp)) {

        read_data(fp);
        word = (address % block_size) / 4;
        idx = (address >> block_two) % set_num;
        cal_tag = address >> (block_two + set_two);

        // write

        if (mode == 'W') {

            // check number existence in cache
            find_cache(cal_tag);

            if (cache_exist == FALSE) {
                find_mem(idx, cal_tag);

                for (j = 0; j < set_size; j++) {
                    if (myCache[idx][j].valid == 0) {
                        load_mem(idx, j);
                        save_cache(idx, j, word);
                        cycle_cnt += 201;
                        break;
                    }
                }

                if (j == set_size) {
                    min_idx = find_seq(idx);
                    if (myCache[idx][min_idx].dirty == 1) {
                        for (k = 0; k < mem_idx; k++) {
                            if (myMemory[k].tag == myCache[idx][min_idx].tag && myMemory[k].idx == idx) {
                                for (int l = 0; l < word_cnt; l++) {
                                    myMemory[k].data[l] = myCache[idx][min_idx].data[l];
                                }
                                break;
                            }
                        }
                        if (k == mem_idx) {
                            myMemory[mem_idx].tag = myCache[idx][min_idx].tag;
                            myMemory[mem_idx].idx = idx;
                            myMemory[mem_idx].data = myCache[idx][min_idx].data[word];
                            mem_idx++;
                        }

                    }

                    load_mem(idx, min_idx);
                    save_cache(idx, min_idx, word);
                    cycle_cnt += 401;

                }
                miss_cnt++;
            }
        }
        // read
        else if (mode == 'R') {
            for (i = 0; i < set_size; i++) {
                if ((cal_tag == myCache[idx][i].tag) && (myCache[idx][i].valid == 1)) {
                    hit_cnt++;
                    cycle_cnt++;
                    break;
                }
            }
            if (i == set_size) {
                find_mem(idx, cal_tag);

                for (j = 0; j < set_size; j++) {
                    if (myCache[idx][j].valid == 0) {
                        load_mem(idx, j);
                        cycle_cnt += 201;
                        break;
                    }
                }
                if (j == set_size) {
                    min_idx = find_seq(idx);
                    if (myCache[idx][min_idx].dirty == 1) {
                        for (k = 0; k < mem_idx; k++) {
                            if (myMemory[k].tag == myCache[idx][min_idx].tag && myMemory[k].idx == idx) {
                                for (int l = 0; l < word_cnt; l++) {
                                    myMemory[k].data[l] = myCache[idx][min_idx].data[l];
                                }
                                break;
                            }
                        }
                        if (k == mem_idx) {
                            myMemory[mem_idx].tag = myCache[idx][min_idx].tag;
                            myMemory[mem_idx].idx = idx;
                            for (int l = 0; l < word_cnt; l++) {
                                myMemory[mem_idx].data[l] = myCache[idx][min_idx].data[l];
                            }

                            mem_idx++;
                        }

                    }

                    load_mem(idx, min_idx);
                    cycle_cnt += 401;
                }
                miss_cnt++;
            }
        }

        if (mem_size == mem_idx + 1) {
            mem_size += 10;
            if (mem_size < pow(2, 32) / word_cnt) {
                myMemory = (mainMemory*)realloc(myMemory, mem_size);
            }
        }

        
    }

    // print result
    for (i = 0; i < set_num; i++) {
        for (int j = 0; j < set_size; j++) {
            if (j == 0) printf("%d: ", i);
            else printf("   ");
            for (int k = word_cnt - 1; k >= 0; k--) {
                printf("%08x ", myCache[i][j].data[k]);
            }
            printf("v:%d d:%d \n", myCache[i][j].valid, myCache[i][j].dirty);
            if (myCache[i][j].dirty == 1) dirty_cnt++;
        }
        printf("\n");
    }

    cycle_cnt = (double)cycle_cnt / (double)(miss_cnt + hit_cnt);
    miss_rate = (double)miss_cnt * 100 / (double)(hit_cnt + miss_cnt);
    printf("total number of hits: %d\ntotal number of misses: %d\n", hit_cnt, miss_cnt);
    printf("miss rate: %.1f%%\n", miss_rate);
    printf("total number of dirty blocks: %d\n", dirty_cnt);
    printf("average memory access cycle: %.1f", cycle_cnt);

    free(myMemory);
    free(myCache);
    free(mem_data);

    return 0;
}

// initialize data
void init() {
    cache_line = cache_size / block_size;
    set_num = cache_line / set_size;
    word_cnt = block_size / 4;

    myCache = (cacheMemory**)malloc(sizeof(cacheMemory*) * set_num);
    for (int i = 0; i < set_num; i++) {
        myCache[i] = (cacheMemory*)malloc(sizeof(cacheMemory) * set_size);
        for (int j = 0; j < set_size; j++) {
            myCache[i][j].data = (int*)malloc(sizeof(int) * word_cnt);
        }

    }

    for (int i = 0; i < set_num; i++) {
        for (int j = 0; j < set_size; j++) {
            myCache[i][j].valid = 0;
            myCache[i][j].dirty = 0;
            myCache[i][j].tag = 0;
            myCache[i][j].seq = 0;
            for (int k = 0; k < word_cnt; k++) {
                myCache[i][j].data[k] = 0;
            }

        }
    }

    myMemory = (mainMemory*)malloc(sizeof(mainMemory) * mem_size);
    for (int i = 0; i < mem_size; i++) {
        myMemory[i].data = (int*)malloc(sizeof(int) * word_cnt);
    }

    for (int i = 0; i < mem_size; i++) {
        for (int j = 0; j < word_cnt; j++) {
            myMemory[i].data[j] = 0;
        }
        myMemory[i].tag = 0;
        myMemory[i].idx = 0;
    }

    mem_data = (int*)malloc(sizeof(int) * word_cnt);
}

void read_data(FILE* fp) {

    fscanf(fp, "%lx", &address);
    fscanf(fp, "%c", &mode);
    if (mode == 'W') {
        fscanf(fp, "%ld", &data);
    }
    else if (mode == 'R') {
        data = 0;
    }

}

// return <log 2 number> value
int logTwo(int number) {
    int cnt = 0;
    while (1) {
        if ((number /= 2) == 0) {
            break;
        }
        else
            cnt++;
    }
    return cnt;
}

void find_cache(int cal_tag) {
    cache_exist = FALSE;
    for (i = 0; i < set_size; i++) {
        if (myCache[idx][i].tag == cal_tag) {
            if (mode == 'W') {
                myCache[idx][i].data[word] = data;
                myCache[idx][i].dirty = 1;
                cache_exist = TRUE;
                hit_cnt++;
                cycle_cnt++;
                break;
            }
        }
    }
}

void find_mem(int idx, int tag) {
    mem_exist = FALSE;
    for (int j = 0; j < mem_idx; j++) {
        if ((myMemory[j].tag == cal_tag) && (myMemory[j].idx == idx)) {
            mem_exist = TRUE;
            for (int k = 0; k < word_cnt; k++) {
                mem_data[k] = myMemory[j].data[k];
            }
            break;
        }
    }
    if (!mem_exist) {
        for (int k = 0; k < word_cnt; k++) {
            mem_data[k] = 0;
        }
    }
}

void save_cache(int idx, int i, int word) {
    myCache[idx][i].data[word] = data;
    myCache[idx][i].dirty = 1;
}

void load_mem(int idx, int j) {
    for (int k = 0; k < word_cnt; k++) {
        myCache[idx][j].data[k] = mem_data[k];
    }
    myCache[idx][j].valid = 1;
    myCache[idx][j].dirty = 0;
    myCache[idx][j].tag = cal_tag;
    myCache[idx][j].seq = seq++;
}

int find_seq(int idx) {
    int min_seq = 999999999;
    int min_idx;
    for (int k = 0; k < set_size; k++) {
        if (myCache[idx][k].seq < min_seq) {
            min_idx = k;
            min_seq = myCache[idx][k].seq;
        }
    }
    return min_idx;
}
