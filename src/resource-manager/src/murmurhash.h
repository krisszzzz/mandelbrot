#if !defined MURMURHASH_INCLUDED

#define MURMURHASH_INCLUDED
#include <stdlib.h>

typedef unsigned long long hash_t;

hash_t MurmurHash(const char* key, size_t data_size);

#endif