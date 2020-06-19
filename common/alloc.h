/*  Gemeral purpose memory allocation macros
 *
 *  alloc.h
 *
 *  Created by Sunrise Telephone Systems KK
 *
 *  This file ("alloc.h") is hereby released into the public domain.
 *
 */

#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

// allocation funtion
#define ALLOCATE(_size) malloc(_size)

// reallocation function
#define REALLOCATE(_pointer, _new_size) realloc(_pointer, _new_size)

// deallocation function
#define DEALLOCATE(_pointer) free(_pointer)

#endif /* ALLOC_H */

// END OF FILE