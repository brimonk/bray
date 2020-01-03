/*
 * Brian Chrzanowski
 * Thu Jan 02, 2020 22:05
 *
 * Common Funcs
 */

#include <stdlib.h>

#include "common.h"

/* C_ArrayRealloc : realloc an array as needed */
void C_ArrayRealloc(void *p, size_t *cnt, size_t *len, size_t elem)
{
	// NOTE (brian)
	// 1. this doesn't handle realloc failures
	// 2. we ASSUME (so the compiler will can it) that you PASS IN a
	//    void **-like thing. You have been warned.

	void **v;

	v = (void **)p;

	if (*cnt == *len) {
		if (*cnt) {
			*cnt *= 2;
		} else {
			*cnt = BUFSMALL;
		}
		*v = realloc(*v, elem * *cnt);
	}
}

