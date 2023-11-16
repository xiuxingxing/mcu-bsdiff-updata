#ifndef __api_lz4
#define __api_lz4

#include "lz4.h"

int lz4_compress_file(char *file_name,char *compress_file_name);
int lz4_decompress_file(char *compress_file_name,char *file_name);




#endif
