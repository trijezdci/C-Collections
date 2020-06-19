/* Bitset Library
 *
 *  @file BITSET.c
 *  Bitset implementation
 *
 *  General purpose bitsets
 *
 *  Author: Benjamin Kowarsch
 *
 *  Copyright (C) 2009 Benjamin Kowarsch. All rights reserved.
 *
 *  License:
 *
 *  Redistribution  and  use  in source  and  binary forms,  with  or  without
 *  modification, are permitted provided that the following conditions are met
 *
 *  1) NO FEES may be charged for the provision of the software.  The software
 *     may  NOT  be published  on websites  that contain  advertising,  unless
 *     specific  prior  written  permission has been obtained.
 *
 *  2) Redistributions  of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  3) Redistributions  in binary form  must  reproduce  the  above  copyright
 *     notice,  this list of conditions  and  the following disclaimer  in the
 *     documentation and other materials provided with the distribution.
 *
 *  4) Neither the author's name nor the names of any contributors may be used
 *     to endorse  or  promote  products  derived  from this software  without
 *     specific prior written permission.
 *
 *  5) Where this list of conditions  or  the following disclaimer, in part or
 *     as a whole is overruled  or  nullified by applicable law, no permission
 *     is granted to use the software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO,  PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA,  OR PROFITS; OR BUSINESS
 * INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY THEORY OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *  
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "BITSET.h"
#include "../common/alloc.h"

// ---------------------------------------------------------------------------
// Integrity checks
// ---------------------------------------------------------------------------

#if (BITSET_MAXIMUM_ELEMENTS < 1)
#error BITSET_MAXIMUM_ELEMENTS must not be less than 1
#endif

#if (BITSET_MAXIMUM_ELEMENTS > 65535)
#error BITSET_MAXIMUM_ELEMENTS must not be greater than 65535
#endif


// ---------------------------------------------------------------------------
// Bitset segmentation
// ---------------------------------------------------------------------------

// IMPLEMENTATION DEPENDENT DATA - DO NOT MODIFY !!!
#define BITSET_BITS_PER_SEGMENT 32

// IMPLEMENTATION DEPENDENT DATA - DO NOT MODIFY !!!
#define BITSET_SEGMENTS_PER_SET \
    ((BITSET_MAXIMUM_ELEMENTS / BITSET_BITS_PER_SEGMENT) + 1)


// ---------------------------------------------------------------------------
// Bitset segment type
// ---------------------------------------------------------------------------

typedef uint_fast32_t bitset_segment_t;


// ---------------------------------------------------------------------------
// Bitset type
// ---------------------------------------------------------------------------
//
// Bitset storage size  =  4 * ((max elements DIV bits per segment) + 1) bytes

typedef bitset_segment_t bitset_a[BITSET_SEGMENTS_PER_SET];


// ---------------------------------------------------------------------------
// Bitset iterator base type
// ---------------------------------------------------------------------------

#if (BITSET_MAXIMUM_ELEMENTS < 256)
#define BITSET_ITERATOR_BASE_TYPE uint8_t
#else
#define BITSET_ITERATOR_BASE_TYPE uint16_t
#endif

typedef BITSET_ITERATOR_BASE_TYPE bitset_iterator_base_t;


// ---------------------------------------------------------------------------
// Bitset iterator type
// ---------------------------------------------------------------------------
//
// Bitset iterator storage size:
//
// o  if max elements > 255:  storage size  =  2 * (max elements + 1) bytes
// o  if max elements < 256:  storage size  =  (max elements + 1) bytes

typedef bitset_iterator_base_t *bitset_iterator_a;


// ---------------------------------------------------------------------------
// Private macros for iterators
// ---------------------------------------------------------------------------

#define _ELEMENT_COUNT(_iterator) _iterator[0]
#define _ELEMENT_AT_INDEX(_iterator, _index) _iterator[_index + 1]


// ---------------------------------------------------------------------------
// function:  set_from_list( list )
// ---------------------------------------------------------------------------
//
// Returns a  new bitset  with the elements passed as parameters included.  At
// least one element must be passed.  Further elements may be passed as varia-
// dic parameters.  The list of elements must always be terminated  by passing
// zero as the last argument in the function call.  Values passed in  that are
// outside of the range of defined elements are ignored.  If zero is passed in
// as the only argument,  an empty bitset is returned.

bitset_t bitset_from_list(bitset_element_t first_element, ...) {
    bitset_a *new_set;
    uint_fast8_t bit, segment = 0;
    bitset_element_t element;
    va_list element_list;
    va_start(element_list, first_element);
    
    new_set = ALLOCATE(sizeof(bitset_a));
    
    if (new_set == NULL) return NULL;
    
    while (segment < BITSET_SEGMENTS_PER_SET) {
        (*new_set)[segment] = 0;
        segment++;
    } // end while
    
    element = first_element;
    
    while (element != 0) {
        if (element < BITSET_MAXIMUM_ELEMENTS) {
            segment = (uint_fast8_t) (element / BITSET_BITS_PER_SEGMENT);
            bit = (uint_fast8_t) (element % BITSET_BITS_PER_SEGMENT);
            (*new_set)[segment] = (*new_set)[segment] | (1 << bit);
        } // end if
        element = va_arg(element_list, bitset_element_t); // next element
    } // end while
    
    return (bitset_t) new_set;
} // end bitset_from_list



// ---------------------------------------------------------------------------
// function:  bitset_has_element( set, element )
// ---------------------------------------------------------------------------
//
// Tests membership of element <element> in bitset <set>.  Returns true if the
// element is a member, elment ∈ set,  returns false otherwise.  If a value is
// passed in that is outside the range of defined tokens,  false is returned.

bool bitset_has_element(bitset_t set, bitset_element_t element) {
    bitset_a *_set = (bitset_a *) set;
    uint_fast8_t segment, bit;
    
    if (element >= BITSET_MAXIMUM_ELEMENTS)
        return false;
    
    segment = (uint_fast8_t) (element / BITSET_BITS_PER_SEGMENT);
    bit = (uint_fast8_t) (element % BITSET_BITS_PER_SEGMENT);
    
    return ((*_set)[segment] & (1 << bit)) != 0;
} // end bitset_has_element


// ---------------------------------------------------------------------------
// function:  bitset_is_subset( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns true if bitset <set2> is a subset of bitset <set1>,  that is if all
// elements of bitset <set2> are also elements of bitset <set1>,  set2 ⊆ set1,
// returns false otherwise.

bool bitset_is_subset(bitset_t set1, bitset_t set2) {
    
    bitset_a *_set1 = (bitset_a *) set1;
    bitset_a *_set2 = (bitset_a *) set2;
    uint_fast8_t segment = 0;
    
    while (segment < BITSET_SEGMENTS_PER_SET) {
        if ((((*_set1)[segment] & (*_set2)[segment]) ^ (*_set2)[segment]) == 0)
            segment++;
        else
            return false;
    } // end while
    
    return true;
} // end bitset_is_subset


// ---------------------------------------------------------------------------
// function:  bitset_is_disjunct( set1, set2 )
// ---------------------------------------------------------------------------
//
// Tests if bitsets <set1> and <set2>  are disjunct,  Returns true if the sets
// are disjunct,  set1 ∩ set2 = {},  returns false otherwise.

bool bitset_is_disjunct(bitset_t set1, bitset_t set2) {
    
    bitset_a *_set1 = (bitset_a *) set1;
    bitset_a *_set2 = (bitset_a *) set2;
    uint_fast8_t segment = 0;

    while (segment < BITSET_SEGMENTS_PER_SET) {
        if (((*_set1)[segment] & (*_set2)[segment]) == 0)
            segment++;
        else
            return false;
    } // end while

    return true;
} // end bitset_is_disjunct


// ---------------------------------------------------------------------------
// function:  bitset_incl( set, element )
// ---------------------------------------------------------------------------
//
// Includes element <element>  in bitset <set>.  Any value  passed in  that is
// outside of the range of defined elements is ignored.

void bitset_incl(bitset_t set, bitset_element_t element) {
    
    bitset_a *_set = (bitset_a *) set;
    uint_fast8_t segment, bit;

    if (element >= BITSET_MAXIMUM_ELEMENTS)
        return;
    
    segment = (uint_fast8_t) (element / BITSET_BITS_PER_SEGMENT);
    bit = (uint_fast8_t) (element % BITSET_BITS_PER_SEGMENT);
    (*_set)[segment] = (*_set)[segment] | (1 << bit);
    
    return;
} // end bitset_incl


// ---------------------------------------------------------------------------
// function:  bitset_excl( set, element )
// ---------------------------------------------------------------------------
//
// Excludes element <element> from bitset <set>.  Any value passed in  that is
// outside of the range of defined elements is ignored.

void bitset_excl(bitset_t set, bitset_element_t element) {
    
    bitset_a *_set = (bitset_a *) set;
    uint_fast8_t segment, bit;
    
    if (element >= BITSET_MAXIMUM_ELEMENTS)
        return;
    
    segment = (uint_fast8_t) (element / BITSET_BITS_PER_SEGMENT);
    bit = (uint_fast8_t) (element % BITSET_BITS_PER_SEGMENT);
    (*_set)[segment] = (*_set)[segment] & ~(1 << bit);
    
    return;
} // end bitset_excl


// ---------------------------------------------------------------------------
// function:  bitset_incl_list( set, element_list )
// ---------------------------------------------------------------------------
//
// Includes the elements passed as variadic parameters from bitset <set>.  The
// list of elements must be terminated by passing zero as the last argument in
// the function call.  Any value  passed in  that is  outside  of the range of
// defined elements is ignored.

void bitset_incl_list(bitset_t set, ...) {
    
    bitset_a *_set = (bitset_a *) set;
    uint_fast8_t segment, bit;
    bitset_element_t element;
    va_list element_list;
    va_start(element_list, set);
    
    element = va_arg(element_list, bitset_element_t);
    
    while (element != 0) {
        if (element < BITSET_MAXIMUM_ELEMENTS) {
            segment = (uint_fast8_t) (element / BITSET_BITS_PER_SEGMENT);
            bit = (uint_fast8_t) (element % BITSET_BITS_PER_SEGMENT);
            (*_set)[segment] = (*_set)[segment] | (1 << bit);
        } // end if
        element = va_arg(element_list, bitset_element_t); // next element
    } // end while
    
    return;
} // end bitset_incl_list


// ---------------------------------------------------------------------------
// function:  bitset_excl_list( set, element_list )
// ---------------------------------------------------------------------------
//
// Excludes the elements passed as variadic parameters from bitset <set>.  The
// list of elements must be terminated by passing zero as the last argument in
// the function call.  Any value  passed in  that is  outside  of the range of
// defined elements is ignored.

void bitset_excl_list(bitset_t set, ...) {
    
    bitset_a *_set = (bitset_a *) set;
    uint_fast8_t segment, bit;
    bitset_element_t element;
    va_list element_list;
    va_start(element_list, set);
    
    element = va_arg(element_list, bitset_element_t);
    
    while (element != 0) {
        if (element < BITSET_MAXIMUM_ELEMENTS) {
            segment = (uint_fast8_t) (element / BITSET_BITS_PER_SEGMENT);
            bit = (uint_fast8_t) (element % BITSET_BITS_PER_SEGMENT);
            (*_set)[segment] = (*_set)[segment] & ~(1 << bit);
        } // end if
        element = va_arg(element_list, bitset_element_t); // next element
    } // end while
    
    return;
} // end bitset_excl_list


// ---------------------------------------------------------------------------
// function:  bitset_union( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns the union of bitsets <set1> and <set2>,  set1 ∪ set2.

bitset_t bitset_union(bitset_t set1, bitset_t set2) {
    
    bitset_a *_set1 = (bitset_a *) set1;
    bitset_a *_set2 = (bitset_a *) set2;
    bitset_a *new_set;
    uint_fast8_t segment = 0;
    
    new_set = ALLOCATE(sizeof(bitset_a));
    
    if (new_set == NULL) return NULL;

    while (segment < BITSET_SEGMENTS_PER_SET) {
        (*new_set)[segment] = (*_set1)[segment] | (*_set2)[segment]; 
        segment++;
    } // end while
    
    return (bitset_t) new_set;
} // end bitset_union


// ---------------------------------------------------------------------------
// function:  bitset_intersection( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns the intersection of bitsets <set1> and <set2>,  set1 ∩ set2.

bitset_t bitset_intersection(bitset_t set1, bitset_t set2) {
    
    bitset_a *_set1 = (bitset_a *) set1;
    bitset_a *_set2 = (bitset_a *) set2;
    bitset_a *new_set;
    uint_fast8_t segment = 0;
    
    new_set = ALLOCATE(sizeof(bitset_a));
    
    if (new_set == NULL) return NULL;
    
    while (segment < BITSET_SEGMENTS_PER_SET) {
        (*new_set)[segment] = (*_set1)[segment] & (*_set2)[segment]; 
        segment++;
    } // end while
    
    return (bitset_t) new_set;    
} // end bitset_intersection


// ---------------------------------------------------------------------------
// function:  bitset_difference( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns the difference of bitsets <set1> and <set2>,  set1 \ set 2.

bitset_t bitset_difference(bitset_t set1, bitset_t set2) {
    
    bitset_a *_set1 = (bitset_a *) set1;
    bitset_a *_set2 = (bitset_a *) set2;
    bitset_a *new_set;
    uint_fast8_t segment = 0;
    
    new_set = ALLOCATE(sizeof(bitset_a));
    
    if (new_set == NULL) return NULL;
    
    while (segment < BITSET_SEGMENTS_PER_SET) {
        (*new_set)[segment] =
        ((*_set1)[segment] | (*_set2)[segment]) & (~ (*_set2)[segment]);
        segment++;
    } // end while
    
    return (bitset_t) new_set;    
} // end bitset_difference


// ---------------------------------------------------------------------------
// function:  bitset_dispose( set )
// ---------------------------------------------------------------------------
//
// Disposes of bitset <set>.

void bitset_dispose(bitset_t set) {
    
    if (set == NULL) return;
    
    DEALLOCATE(set);
} // end bitset_dispose


// ---------------------------------------------------------------------------
// function:  bitset_iterator_from_set( set )
// ---------------------------------------------------------------------------
//
// Returns a new iterator for bitset <set>.

bitset_iterator_t bitset_iterator_from_set(bitset_t set) {
    bitset_a *_set = (bitset_a *) set;
    bitset_element_t element_list[BITSET_MAXIMUM_ELEMENTS];
    bitset_element_t element = 0;
    cardinal index = 0;
    bitset_iterator_base_t *new_iterator;
    uint_fast8_t bit, segment;
    
    if (set == NULL) return NULL;
    
    bit = 0;
    segment = 0;
    while (segment < BITSET_SEGMENTS_PER_SET) {
        while (bit < BITSET_BITS_PER_SEGMENT) {
            if (((*_set)[segment] & (1 << bit)) != 0) {
                element_list[index] = element;
                index++;
            } // end if
            element++;
            bit++;
        } // end while
        bit = 0;
        segment++;
    } // end while
    
    new_iterator = ALLOCATE(index * sizeof(bitset_iterator_base_t));
    
    if (new_iterator == NULL) return NULL;
    
    _ELEMENT_COUNT(new_iterator) = index;
    
    index = 0;
    while (index < _ELEMENT_COUNT(new_iterator)) {
        _ELEMENT_AT_INDEX(new_iterator, index) = element_list[index];
        index++;
    } // end while
    
    return (bitset_iterator_t) new_iterator;
} // end bitset_iterator


// ---------------------------------------------------------------------------
// function:  bitset_iterator_element_count( iterator )
// ---------------------------------------------------------------------------
//
// Returns the number of elements in bitset iterator <iterator>.

cardinal bitset_iterator_element_count(bitset_iterator_t iterator) {
    bitset_iterator_a _iterator = (bitset_iterator_a) iterator;

    if (iterator == NULL) return 0;
    
    return _ELEMENT_COUNT(_iterator);
} // end bitset_iterator_element_count


// ---------------------------------------------------------------------------
// function:  bitset_iterator_element_at_index( iterator, index )
// ---------------------------------------------------------------------------
//
// Returns the element at index <index> in bitset iterator <iterator>.

bitset_element_t bitset_iterator_element_at_index(bitset_iterator_t iterator,
                                                  cardinal index) {
    bitset_iterator_a _iterator = (bitset_iterator_a) iterator;
    
    if ((iterator == NULL) || (index >= _ELEMENT_COUNT(_iterator))) return 0;
    
    return _ELEMENT_AT_INDEX(_iterator, index);
} // end bitset_iterator_element_at_index


// ---------------------------------------------------------------------------
// function:  bitset_iterator_dispose( iterator )
// ---------------------------------------------------------------------------
//
// Disposes of bitset iterator <iterator>.

void bitset_iterator_dispose(bitset_iterator_t iterator) {
    
    if (iterator == NULL) return;
    
    DEALLOCATE(iterator);
} // end bitset_iterator_dispose


// END OF FILE
