/* mempoke.c */

#include <string.h>
#include "memory.h"

/* Function: pj_stuff_bytes */
void
pj_stuff_bytes(uint8_t data, void *dst, unsigned int n)
{
	memset(dst, data, n);
}

/* Function: pj_copy_bytes */
void
pj_copy_bytes(const void *src, void *dst, unsigned int n)
{
	memmove(dst, src, n);
}

/* Function: pj_copy_words */
void
pj_copy_words(const void *src, void *dst, unsigned int n)
{
	memmove(dst, src, 2 * n);
}

/* Function: pj_copy_structure */
void
pj_copy_structure(const void *src, void *dst, unsigned int n)
{
	memmove(dst, src, n);
}