/* LIFO Storage Library
 *
 *  @file LIFO.h
 *  LIFO interface
 *
 *  Universal Dynamic Stack
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


#ifndef LIFO_H
#define LIFO_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Default stack size
// ---------------------------------------------------------------------------

#define LIFO_DEFAULT_STACK_SIZE 256


// ---------------------------------------------------------------------------
// Maximum stack size
// ---------------------------------------------------------------------------

#define LIFO_MAXIMUM_STACK_SIZE 0xffffffff  /* more than 2 billion entries */


// ---------------------------------------------------------------------------
// Determine type to hold stack size values
// ---------------------------------------------------------------------------

#if (LIFO_MAXIMUM_STACK_SIZE <= ((1 << 8) - 1))
#define LIFO_SIZE_BASE_TYPE uint8_t
#elif (LIFO_MAXIMUM_STACK_SIZE <= ((1 << 16) - 1))
#define LIFO_SIZE_BASE_TYPE uint16_t
#elif (LIFO_MAXIMUM_STACK_SIZE <= ((1 << 32) - 1))
#define LIFO_SIZE_BASE_TYPE uint32_t
#elif (LIFO_MAXIMUM_STACK_SIZE <= ((1 << 64UL) - 1))
#define LIFO_SIZE_BASE_TYPE uint64_t
#else
#error LIFO_MAXIMUM_STACK_SIZE out of range
#endif


// ---------------------------------------------------------------------------
// Opaque LIFO handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t lifo_t;


// ---------------------------------------------------------------------------
// LIFO stack size type
// ---------------------------------------------------------------------------

typedef LIFO_SIZE_BASE_TYPE lifo_size_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *lifo_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* lifo_status_t */ {
    LIFO_STATUS_SUCCESS = 1,
    LIFO_STATUS_INVALID_SIZE,
    LIFO_STATUS_INVALID_STACK,
    LIFO_STATUS_INVALID_DATA,
    LIFO_STATUS_STACK_OVERFLOW,
    LIFO_STATUS_STACK_EMPTY,
    LIFO_STATUS_ALLOCATION_FAILED
} lifo_status_t;


// ---------------------------------------------------------------------------
// function:  lifo_new_stack( initial_size, status )
// ---------------------------------------------------------------------------
//
// Creates  and  returns  a new LIFO stack object  with an initial capacity of
// <initial_size>.  If  zero  is passed in for <initial_size>,  then the stack
// will be created  with an initial capacity of  LIFO_DEFAULT_STACK_SIZE.  The
// function fails if a value greater than LIFO_MAXIMUM_STACK_SIZE is passed in
// for <initial_size> or if memory could not be allocated.
//
// The initial capacity of a stack is the number of entries that can be stored
// in the stack without enlargement.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

lifo_t lifo_new_stack(lifo_size_t initial_size, lifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  lifo_push( stack, value, status )
// ---------------------------------------------------------------------------
//
// Adds a  new entry <value>  to the top of stack <stack>.  The  new entry  is
// added by reference,  no data is copied.  However,  no entry is added if the
// the stack is full,  that is  when the number of entries stored in the stack
// has reached LIFO_MAXIMUM_STACK_SIZE.  The function fails  if NULL is passed
// in for <stack> or <value>,  or if memory allocation failed.
//
// New entries are allocated dynamically  if the number of entries exceeds the
// initial capacity of the stack.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void lifo_push(lifo_t stack, lifo_data_t value, lifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  lifo_pop( stack, status )
// ---------------------------------------------------------------------------
//
// Removes the top most value from stack <stack> and returns it.  If the stack
// is empty,  that  is  when the  number  of  entries  stored in the stack has
// reached zero,  then NULL is returned.
//
// Entries which were allocated dynamically (above the initial capacity) are
// deallocated when their values are popped.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

lifo_data_t lifo_pop(lifo_t stack, lifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  lifo_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns the current capacity of <stack>.  The current capacity is the total
// number of allocated entries. Returns zero if NULL is passed in for <stack>.

lifo_size_t lifo_stack_size(lifo_t stack);


// ---------------------------------------------------------------------------
// function:  lifo_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the  number of entries  stored in stack <stack>,  returns  zero if
// NULL is passed in for <stack>.

lifo_size_t lifo_number_of_entries(lifo_t stack);


// ---------------------------------------------------------------------------
// function:  lifo_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of LIFO stack object <stack>.  Returns NULL.

lifo_t lifo_dispose_stack(lifo_t stack);


#endif /* LIFO_H */

// END OF FILE
