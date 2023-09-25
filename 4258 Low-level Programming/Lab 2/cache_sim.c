#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum { dm, fa } cache_map_t;
typedef enum { uc, sc } cache_org_t;
typedef enum { instruction, data } access_t;

typedef struct {
  uint32_t address;
  access_t accesstype;
} mem_access_t;

typedef struct {
  uint64_t accesses;
  uint64_t hits;
  // You can declare additional statistics if
  // you like, however you are now allowed to
  // remove the accesses or hits
} cache_stat_t;

// DECLARE CACHES AND COUNTERS FOR THE STATS HERE

uint32_t cache_size;
uint32_t block_size = 64;
cache_map_t cache_mapping;
cache_org_t cache_org;

uint32_t nb_set;
uint32_t nb_way;
uint32_t bit_offset;
uint32_t bit_index;
uint32_t bit_tag;

uint32_t address_tag;
uint32_t previous_address;
uint32_t previous_D_address;
uint32_t previous_I_address;

// USE THIS FOR YOUR CACHE STATISTICS
cache_stat_t cache_statistics;

/* Reads a memory access from the trace file and returns
 * 1) access type (instruction or data access)
 * 2) memory address
 */
mem_access_t read_transaction(FILE* ptr_file) {
  char type;
  mem_access_t access;

  if (fscanf(ptr_file, "%c %x\n", &type, &access.address) == 2) {
    if (type != 'I' && type != 'D') {
      printf("Unkown access type\n");
      exit(0);
    }
    access.accesstype = (type == 'I') ? instruction : data;
    return access;
  }

  /* If there are no more entries in the file,
   * return an address 0 that will terminate the infinite loop in main
   */
  access.address = 0;
  return access;
}

void main(int argc, char** argv) {
  // Reset statistics:
  memset(&cache_statistics, 0, sizeof(cache_stat_t));

  /* Read command-line parameters and initialize:
   * cache_size, cache_mapping and cache_org variables
   */
  /* IMPORTANT: *IF* YOU ADD COMMAND LINE PARAMETERS (you really don't need to),
   * MAKE SURE TO ADD THEM IN THE END AND CHOOSE SENSIBLE DEFAULTS SUCH THAT WE
   * CAN RUN THE RESULTING BINARY WITHOUT HAVING TO SUPPLY MORE PARAMETERS THAN
   * SPECIFIED IN THE UNMODIFIED FILE (cache_size, cache_mapping and cache_org)
   */
  if (argc != 4) { /* argc should be 2 for correct execution */
    printf(
        "Usage: ./cache_sim [cache size: 128-4096] [cache mapping: dm|fa] "
        "[cache organization: uc|sc]\n");
    exit(0);
  } else {
    /* argv[0] is program name, parameters start with argv[1] */

    /* Set cache size */
    cache_size = atoi(argv[1]);

    /* Set Cache Mapping */
    if (strcmp(argv[2], "dm") == 0) {
      cache_mapping = dm;
    } else if (strcmp(argv[2], "fa") == 0) {
      cache_mapping = fa;
    } else {
      printf("Unknown cache mapping\n");
      exit(0);
    }

    /* Set Cache Organization */
    if (strcmp(argv[3], "uc") == 0) {
      cache_org = uc;
    } else if (strcmp(argv[3], "sc") == 0) {
      cache_org = sc;
    } else {
      printf("Unknown cache organization\n");
      exit(0);
    }
  }

  /* Open the file mem_trace.txt to read memory accesses */
  FILE* ptr_file;
  ptr_file = fopen("mem_trace2.txt", "r");
  if (!ptr_file) {
    printf("Unable to open the trace file\n");
    exit(1);
  }


  // Intialization of the variable with the correct number in it
  if (!cache_org && cache_mapping ) {      // fa uc
    nb_set = 1;
    nb_way = cache_size/block_size;
    bit_offset =  log2(block_size);
    bit_index = 0;
    bit_tag = 32 - bit_offset - bit_index;

    
  }
  if (cache_org && cache_mapping) {       // fa sc
    cache_size = cache_size/2;
    nb_set = 1;
    nb_way = cache_size/block_size;
    bit_offset =  log2(block_size);
    bit_index = 0;
    bit_tag = 32 - bit_offset - bit_index;
  }
  if (!cache_org  && !cache_mapping) {    // dm uc
    nb_set = cache_size/block_size;
    nb_way = 1;
    bit_offset =  log2(block_size);
    bit_index = log2(nb_set);
    bit_tag = 32 - bit_offset - bit_index;
  }
  if (cache_org && !cache_mapping) {      // dm sc
    cache_size = cache_size/2;
    nb_set = cache_size/block_size;
    nb_way = 1;
    bit_offset =  log2(block_size);
    bit_index = log2(nb_set);
    bit_tag = 32 - bit_offset - bit_index;
  }

  uint32_t address_list[nb_way];
  uint32_t address_I_list[nb_way];
  uint32_t address_D_list[nb_way];


  /* Loop until whole trace file has been read */
  mem_access_t access;
  while (1) {
    access = read_transaction(ptr_file);
    // If no transactions left, break out of loop
    if (access.address == 0) break;

    address_tag = access.address >> (32 - bit_tag);     
    
    //printf("%d %x %x %x\n", access.accesstype, access.address, address_tag, previous_address);
    /* Do a cache access */
    // 0 -> I -> Instruction
    // 1 -> D -> Data
    // ADD YOUR CODE HERE
    cache_statistics.accesses++;   // the accesses are increase each time we have an instruction or a data

    if (!cache_org && cache_mapping ) {     // fa uc
      int bool_hit = 0;
      for(int k=0; k<nb_way ; k++){
        if (address_tag == address_list[k]){
          cache_statistics.hits++;
          bool_hit = 1;
        }
      }
      if (!bool_hit){         // if this is a miss
        for(int k=1; k<nb_way; k++){
          address_list[k-1] = address_list[k];
        }
        address_list[nb_way - 1] = address_tag;
      }
    }


    if (cache_org && cache_mapping) {       // fa sc
      if (access.accesstype){     // for the data
        int bool_hit = 0;
        for(int k=0; k<nb_way ; k++){
          if (address_tag == address_D_list[k]){
            cache_statistics.hits++;
            bool_hit = 1;
          }
        }
        if (!bool_hit){         // if this is a miss
          for(int k=1; k<nb_way; k++){
            address_D_list[k-1] = address_D_list[k];
          }
          address_D_list[nb_way - 1] = address_tag;
        }
      }
      if (!access.accesstype){     // for the instruction
        int bool_hit = 0;
        for(int k=0; k<nb_way ; k++){
          if (address_tag == address_I_list[k]){
            cache_statistics.hits++;
            bool_hit = 1;
          }
        }
        if (!bool_hit){         // if this is a miss
          for(int k=1; k<nb_way; k++){
            address_I_list[k-1] = address_I_list[k];
          }
          address_I_list[nb_way - 1] = address_tag;
        }
      }
    }


    if (!cache_org && !cache_mapping) {      // dm uc
      if (address_tag == previous_address){
        cache_statistics.hits++;
      }
      previous_address = address_tag;
    }


    if (cache_org  && !cache_mapping) {     // dm sc
      if (access.accesstype){               // for the data
        if (address_tag == previous_D_address){
          cache_statistics.hits++;
        }
        previous_D_address = address_tag;
      }
      if (!access.accesstype){              // for the instruction
        if (address_tag == previous_I_address){
          cache_statistics.hits++;
        }
        previous_I_address = address_tag;
      }
    }

  }
  /* Print the statistics */
  // DO NOT CHANGE THE FOLLOWING LINES!
  printf("\nCache Statistics\n");
  printf("-----------------\n\n");
  printf("Accesses: %ld\n", cache_statistics.accesses);
  printf("Hits:     %ld\n", cache_statistics.hits);
  printf("Hit Rate: %.4f\n",
         (double)cache_statistics.hits / cache_statistics.accesses);
  // DO NOT CHANGE UNTIL HERE
  // You can extend the memory statistic printing if you like!

  printf("-----------------\n\n");
  printf("Size:   %d\n", cache_size);
  printf("Set:    %d\n", nb_set);
  printf("Way:    %d\n", nb_way);
  printf("Offset: %d\n", bit_offset);
  printf("Index:  %d\n", bit_index);
  printf("Tag:    %d\n", bit_tag);

  /* Close the trace file */
  fclose(ptr_file);
}

