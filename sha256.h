#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>

void sha256(const unsigned char *data, size_t len, unsigned char hash[32]);


#endif
