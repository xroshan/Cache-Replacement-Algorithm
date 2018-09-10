#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/cache.h"

typedef struct {
  int data_points;
  int c;
  int Lhirs_size;
  int algo;
  char fname[1024];
  char mname[1024];
  FILE* trace;
  FILE* mmap;
} input;

int usage(char* bin) {
  printf(
      "USAGE: %s <hir> <#> <trace> <type>\n"
      "  hir - hir rate out of 100%%\n"
      "  # - # of data points\n"
      "  trace - location of trace file\n"
      "  type - 1 (lru only), 2 (arc only), 3 (lirs only), 4 (all)\n",
      bin);
  return 2;
}

int get_max_capacity(input* in) {
  int size = 0;
  fseek(in->trace, -sizeof(int), SEEK_END);
  fread(&size, sizeof(int), 1, in->trace);
  return size;
}

/*
char* get_out_file_name(input *in)
{

  // truncate fname
  char *token;
  char *string = malloc(strlen(in->fname) + 1);

  if (!string) {
    printf("String allocation failed! exiting\n");
    exit(0);
  }

  strcpy(string, in->fname);

  while ((token = strsep(&string, "/")) != NULL);

  printf("token: %s", token);

  // file name length
  int flen = snprintf(NULL, 0, "%d_%s_%dsz_%dhir_mmap.dat", in->algo, token,
in->c, in->Lhirs_size);

  char* mfname = (char*)malloc(flen + 1);
  snprintf(mfname, flen + 1, "%d_%s_%dsz_%dhir_mmap.dat", in->algo, token,
in->c, in->Lhirs_size);

  // return mfname;
  return mfname;
}
*/

int parse_args(int argc, char** argv, input* in) {
  if (argc < 5) {
    printf("Error: not enough parameters passed\n");
    return usage(argv[0]);
  }

  // check first parameter is an integer
  if (sscanf(argv[1], "%d", &in->Lhirs_size) != 1) {
    printf("Error: HIR in not an integer\n");
    return usage(argv[0]);
  }

  // check second parameter is an integer
  if (sscanf(argv[2], "%d", &in->data_points) != 1) {
    printf("Error: data_points in not an integer\n");
    return usage(argv[0]);
  }

  // check trace file by opening it
  in->trace = fopen(argv[3], "r");
  if (in->trace == NULL) {
    printf("Error: \"%s\" is not a valid path\n", argv[3]);
    return usage(argv[0]);
  }
  strcpy(in->fname, argv[3]);

  if (sscanf(argv[4], "%d", &in->algo) != 1) {
    printf("Error: algo in not an integer\n");
    return usage(argv[0]);
  }

  if (argv[5]) {
    strcpy(in->mname, argv[5]);
  } else {
    in->mname[0] = '\0';
  }

  return 0;
}

void get_out_file_name(input* in) {
  char res[250];
  char num[15];
  switch (in->algo) {
    case 1:
      strcpy(res, "lru_");
      break;
    case 2:
      strcpy(res, "arc_");
      break;
    default:
      strcpy(res, "lirs_");
      break;
  }

  if (in->mname[0]) {
    strcat(res, in->mname);
    strcat(res, "_");
  }

  sprintf(num,"%d", in->c);

  strcat(res, num);
  strcat(res, "_");

  sprintf(num,"%d", in->Lhirs_size);

  strcat(res, num);
  strcat(res, "hir_mmap.dat");

  // fprintf(stderr, "outfile: %s\n", res);

  in->mmap = fopen(res, "w");

  if (!in->mmap)
    exit(0);

}

void trace(input* in) {
  int lines = 0;
  char line[300];
  int i = 0;

  cache* lru = NULL;
  cache* arc = NULL;
  cache* lirs = NULL;

  // for memory map, result of get, ie hit or miss
  int res;

   get_out_file_name(in);

  // fprintf(stderr, "outfile: %s", mfname);

  // FILE* mfile = fopen(mfname, "w");

  // /*
  //   if (in->mname[0])
  //     mfile = fopen(in->mname+in->c, "w");
  //   else
  //     mfile = fopen("mmap_"+in->c, "w");
  // */
  // if (mfile == NULL) {
  //   fprintf(stderr,
  //           "MEMORY MAP COULDN'T be created, write file failed to open!\n");

  //   exit(0);
  // }

  switch (in->algo) {
    case 1:
      lru = cache_create(LRU, in->c, in->Lhirs_size);
      break;
    case 2:
      arc = cache_create(ARC, in->c, in->Lhirs_size);
      break;
    case 3:
      lirs = cache_create(LIRS, in->c, in->Lhirs_size);
      break;
    default:
      lru = cache_create(LRU, in->c, in->Lhirs_size);
      arc = cache_create(ARC, in->c, in->Lhirs_size);
      lirs = cache_create(LIRS, in->c, in->Lhirs_size);
      break;
  }

  while (fread(&i, sizeof(int), 1, in->trace) > 0) {
    switch (in->algo) {
      case 1:
        res = cache_get(lru, i);
        break;
      case 2:
        res = cache_get(arc, i);
        break;
      case 3:
        res = cache_get(lirs, i);
        break;
      default:
        cache_get(lru, i);
        cache_get(arc, i);
        res = cache_get(lirs, i);
        break;
    }

    fprintf(in->mmap, "%d %d %d\n", lines, i, res);

    lines++;
    if (lines % 100000 == 0) {
      fprintf(stderr, "%s(c=%d): processed %d lines\r", in->fname, in->c,
              lines);
      fflush(stderr);

      // use printf to get the type of output you want during caching
      // printf("%d %d %d %d %5.2f\n", lines, lirs->data.lirs->Lsize,
      // lirs->data.lirs->Hsize, lirs->data.lirs->NRsize, lirs->hits * 100 /
      // (float)lirs->requests);
    }

  }

  // use printf to get the type of output you want at the end of caching

  switch (in->algo) {
    case 1:
      // printf("file: %17s, capacity: %6d, algo: lru, ", in->fname, in->c);
      // cache_print_stats(lru);
      cache_free(lru);
      break;

    case 2:
      // printf("file: %17s, capacity: %6d, algo: arc, ", in->fname, in->c);
      // cache_print_stats(arc);
      cache_free(arc);
      break;

    case 3:
      // printf("file: %17s, capacity: %6d, algo: lirs, ", in->fname, in->c);
      // cache_print_stats(lirs);

      printf("%5.2f\n", 100 - lirs->hits / (float)lirs->requests * 100);

      // printf("%d %d %d %d %5.2f\n", in->Lhirs_size, lirs->data.lirs->Lsize,
      // lirs->data.lirs->Hsize, lirs->data.lirs->NRsize, lirs->hits * 100 /
      // (float)lirs->requests); printf("%d %5.2f\n",in->Lhirs_size, lirs->hits *
      // 100 / (float)lirs->requests);

      cache_free(lirs);
      break;

    default:
      // printf("file: %17s, capacity: %6d, algo: lru, ", in->fname, in->c);
      // cache_print_stats(lru);
      // printf("file: %17s, capacity: %6d, algo: arc, ", in->fname, in->c);
      // cache_print_stats(arc);
      // printf("file: %17s, capacity: %6d, algo: lirs, ", in->fname, in->c);
      // cache_print_stats(lirs);

      printf("%d %5.2f %5.2f %5.2f\n", in->c,
             100 - lru->hits / (float)lru->requests * 100,
             100 - arc->hits / (float)arc->requests * 100,
             100 - lirs->hits / (float)lirs->requests * 100);

      cache_free(lru);
      cache_free(arc);
      cache_free(lirs);
      break;
  }
}

int main(int argc, char** argv) {
  input in;

  if (parse_args(argc, argv, &in)) {
    fprintf(stderr, "Parse failed!");
    return 1;
  }

  in.c = 100;
  int max_cache_size = get_max_capacity(&in);
  int increment = (int)max_cache_size / in.data_points;

  fprintf(stderr, "WHILE STARTING!, %d\n", max_cache_size);
  while (in.c <= max_cache_size) {
    fseek(in.trace, 0, SEEK_SET);
    trace(&in);
    in.c += increment;
  }
  fclose(in.trace);
}
