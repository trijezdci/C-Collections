/* Bitset Library
 *
 *  @file BITSET.h
 *  Bitset interface
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


#ifndef BITSET_H
#define BITSET_H

#include "../common/common.h"


// ---------------------------------------------------------------------------
// Maximum number of elements in a bitset
// ---------------------------------------------------------------------------
//
// A value  between 1 and 65535  that determines the capacity and storage size
// of bitsets and bitset iterators.  It may be changed at compile time only.

#define BITSET_MAXIMUM_ELEMENTS 256


// --------------------------------------------------------------------------
// Opaque bitset handle type
// --------------------------------------------------------------------------
//
// WARNING: Objects of this opaque type should only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE. Accessing the internal data structure directly
// other than through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t bitset_t;


// ---------------------------------------------------------------------------
// Opaque set iterator handle type
// ---------------------------------------------------------------------------
//
// WARNING: Objects of this opaque type should only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE. Accessing the internal data structure directly
// other than through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t bitset_iterator_t;


// ---------------------------------------------------------------------------
// Bitset element type
// ---------------------------------------------------------------------------

typedef int bitset_element_t;


// ---------------------------------------------------------------------------
// Empty bitset
// ---------------------------------------------------------------------------
//
// Returns an empty bitset.

#define BITSET_EMPTY_SET bitset_from_list(0)


// ---------------------------------------------------------------------------
// function:  bitset_from_list( list )
// ---------------------------------------------------------------------------
//
// Returns a  new bitset  with the elements passed as parameters included.  At
// least one element must be passed.  Further elements may be passed as varia-
// dic parameters.  The list of elements must always be terminated  by passing
// zero as the last argument in the function call.  Values passed in  that are
// outside of the range of defined elements are ignored.  If zero is passed in
// as the only argument,  an empty bitset is returned.

bitset_t bitset_from_list(bitset_element_t first_element, ...);


// ---------------------------------------------------------------------------
// function:  bitset_has_element( set, element )
// ---------------------------------------------------------------------------
//
// Tests membership of element <element> in bitset <set>.  Returns true if the
// element is a member, elment ∈ set,  returns false otherwise.  If a value is
// passed in that is outside the range of defined tokens,  false is returned.

bool bitset_has_element(bitset_t set, bitset_element_t element);


// ---------------------------------------------------------------------------
// function:  bitset_is_subset( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns true if bitset <set2> is a subset of bitset <set1>,  that is if all
// elements of bitset <set2> are also elements of bitset <set1>,  set2 ⊆ set1,
// returns false otherwise.

bool bitset_is_subset(bitset_t set1, bitset_t set2);


// ---------------------------------------------------------------------------
// function:  bitset_is_disjunct( set1, set2 )
// ---------------------------------------------------------------------------
//
// Tests if bitsets <set1> and <set2>  are disjunct,  Returns true if the sets
// are disjunct,  set1 ∩ set2 = {},  returns false otherwise.

bool bitset_is_disjunct(bitset_t set1, bitset_t set2);


// ---------------------------------------------------------------------------
// function:  bitset_incl( set, element )
// ---------------------------------------------------------------------------
//
// Excludes element <element>  in bitset <set>.  Any value  passed in  that is
// outside of the range of defined elements is ignored.

void bitset_incl(bitset_t set, bitset_element_t element);


// ---------------------------------------------------------------------------
// function:  bitset_excl( set, element )
// ---------------------------------------------------------------------------
//
// Excludes element <element> from bitset <set>.  Any value passed in  that is
// outside of the range of defined elements is ignored.

void bitset_excl(bitset_t set, bitset_element_t element);


// ---------------------------------------------------------------------------
// function:  bitset_incl_list( set, element_list )
// ---------------------------------------------------------------------------
//
// Includes the elements passed as variadic parameters from bitset <set>.  The
// list of elements must be terminated by passing zero as the last argument in
// the function call.  Any value  passed in  that is  outside  of the range of
// defined elements is ignored.

void bitset_incl_list(bitset_t set, ...);


// ---------------------------------------------------------------------------
// function:  bitset_excl_list( set, element_list )
// ---------------------------------------------------------------------------
//
// Excludes the elements passed as variadic parameters from bitset <set>.  The
// list of elements must be terminated by passing zero as the last argument in
// the function call.  Any value  passed in  that is  outside  of the range of
// defined elements is ignored.

void bitset_excl_list(bitset_t set, ...);


// ---------------------------------------------------------------------------
// function:  bitset_union( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns the union of bitsets <set1> and <set2>,  set1 ∪ set2.

bitset_t bitset_union(bitset_t set1, bitset_t set2);


// ---------------------------------------------------------------------------
// function:  bitset_intersection( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns the intersection of bitsets <set1> and <set2>,  set1 ∩ set2.

bitset_t bitset_intersection(bitset_t set1, bitset_t set2);


// ---------------------------------------------------------------------------
// function:  bitset_difference( set1, set2 )
// ---------------------------------------------------------------------------
//
// Returns the difference of bitsets <set1> and <set2>,  set1 \ set 2.

bitset_t bitset_difference(bitset_t set1, bitset_t set2);


// ---------------------------------------------------------------------------
// function:  bitset_dispose( set )
// ---------------------------------------------------------------------------
//
// Disposes of bitset <set>.

void bitset_dispose(bitset_t set);


// ---------------------------------------------------------------------------
// function:  bitset_iterator_from_set( set )
// ---------------------------------------------------------------------------
//
// Returns a new iterator for bitset <set>.

bitset_iterator_t bitset_iterator_from_set(bitset_t set);


// ---------------------------------------------------------------------------
// function:  bitset_iterator_element_count( iterator )
// ---------------------------------------------------------------------------
//
// Returns the number of elements in bitset iterator <iterator>.

cardinal bitset_iterator_element_count(bitset_iterator_t iterator);


// ---------------------------------------------------------------------------
// function:  bitset_iterator_element_at_index( iterator, index )
// ---------------------------------------------------------------------------
//
// Returns the element at index <index> in bitset iterator <iterator>.

bitset_element_t bitset_iterator_element_at_index(bitset_iterator_t iterator,
                                                           cardinal index);


// ---------------------------------------------------------------------------
// function:  bitset_iterator_dispose( iterator )
// ---------------------------------------------------------------------------
//
// Disposes of bitset iterator <iterator>.

void bitset_iterator_dispose(bitset_iterator_t iterator);


#endif /* BITSET_H */

// END OF FILE
