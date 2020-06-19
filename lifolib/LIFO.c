/* LIFO Storage Library
 *
 *  @file LIFO.c
 *  LIFO implementation
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


// ---------------------------------------------------------------------------
// Imports
// ---------------------------------------------------------------------------

#include "LIFO.h"
#include "../common/alloc.h"
#include "../common/common.h"


// ---------------------------------------------------------------------------
// Range checks
// ---------------------------------------------------------------------------

#if (LIFO_DEFAULT_STACK_SIZE < 1)
#error LIFO_DEFAULT_STACK_SIZE must not be zero, recommended minimum is 8
#elif (LIFO_DEFAULT_STACK_SIZE > LIFO_MAXIMUM_STACK_SIZE)
#error LIFO_DEFAULT_STACK_SIZE must not be larger than \
LIFO_MAXIMUM_STACK_SIZE
#endif

#if (LIFO_DEFAULT_STACK_SIZE < 8)
#warning LIFO_DEFAULT_STACK_SIZE is unreasonably low, factory setting is 256
#elif (LIFO_DEFAULT_STACK_SIZE > 65535)
#warning LIFO_DEFAULT_STACK_SIZE is unreasonably high, factory setting is 256
#endif


// ---------------------------------------------------------------------------
// LIFO entry pointer type for self referencing declaration of entry
// ---------------------------------------------------------------------------

struct _lifo_entry_s; /* FORWARD */

typedef struct _lifo_entry_s *lifo_entry_p;


// ---------------------------------------------------------------------------
// LIFO stack entry type
// ---------------------------------------------------------------------------

struct _lifo_entry_s {
    lifo_data_t value;
    lifo_entry_p next;
};

typedef struct _lifo_entry_s lifo_entry_s;


// ---------------------------------------------------------------------------
// LIFO stack type
// ---------------------------------------------------------------------------

typedef struct /* lifo_s */ {
    lifo_entry_s *overflow;
     lifo_size_t entry_count;
     lifo_size_t array_size;
     lifo_data_t value[0];
} lifo_s;


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

lifo_t lifo_new_stack(lifo_size_t initial_size, lifo_status_t *status) {
    lifo_s *stack;
    
    // zero size means default
    if (initial_size == 0) {
        initial_size = LIFO_DEFAULT_STACK_SIZE;
    } // end if
    
    // bail out if initial size is too high
    if (initial_size > LIFO_MAXIMUM_STACK_SIZE) {
        ASSIGN_BY_REF(status, LIFO_STATUS_INVALID_SIZE);
        return NULL;
    } // end if
    
    // allocate new stack
    stack = ALLOCATE(sizeof(lifo_s) + initial_size * sizeof(lifo_data_t));
    
    // bail out if allocation failed
    if (stack == NULL) {
        ASSIGN_BY_REF(status, LIFO_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise meta data
    stack->array_size = initial_size;
    stack->entry_count = 0;
    stack->overflow = NULL;
        
    // pass status and new stack object to caller
    ASSIGN_BY_REF(status, LIFO_STATUS_SUCCESS);
    return (lifo_t) stack;
} // end lifo_new_stack


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

void lifo_push(lifo_t stack, lifo_data_t value, lifo_status_t *status) {
    #define this_stack ((lifo_s *)stack)
    lifo_entry_s *new_entry;
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, LIFO_STATUS_INVALID_STACK);
        return;
    } // end if
    
    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, LIFO_STATUS_INVALID_DATA);
        return;
    } // end if
    
    // bail out if stack is full
    if (this_stack->entry_count >= LIFO_MAXIMUM_STACK_SIZE) {
        ASSIGN_BY_REF(status, LIFO_STATUS_STACK_OVERFLOW);
        return;
    } // end if
    
    // check if index falls within array segment
    if (this_stack->entry_count < this_stack->array_size) {
        
        // store value in array segment
        this_stack->value[this_stack->entry_count] = value;
    }
    else /* index falls within overflow segment */ {
        
        // allocate new entry slot
        new_entry = ALLOCATE(sizeof(lifo_entry_s));
        
        // bail out if allocation failed
        if (new_entry == NULL) {
            ASSIGN_BY_REF(status, LIFO_STATUS_ALLOCATION_FAILED);
            return;
        } // end if
        
        // initialise new entry
        new_entry->value = value;
        
        // link new entry into overflow list
        new_entry->next = this_stack->overflow;
        this_stack->overflow = new_entry;
    } // end if
    
    // updare entry counter
    this_stack->entry_count++;
    
    // pass status to caller
    ASSIGN_BY_REF(status, LIFO_STATUS_SUCCESS);
    return;
    
    #undef this_stack
} // end lifo_push


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

lifo_data_t lifo_pop(lifo_t stack, lifo_status_t *status) {
    #define this_stack ((lifo_s *)stack)
    lifo_entry_s *this_entry;
    lifo_data_t this_value;
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, LIFO_STATUS_INVALID_STACK);
        return NULL;
    } // end if
    
    // bail out if stack is empty
    if (this_stack->entry_count == 0) {
        ASSIGN_BY_REF(status, LIFO_STATUS_STACK_EMPTY);
        return NULL;
    } // end if
    
    this_stack->entry_count--;
    
    // check if index falls within array segment
    if (this_stack->entry_count < this_stack->array_size) {
        
        // return value and status to caller
        ASSIGN_BY_REF(status, LIFO_STATUS_SUCCESS);
        return this_stack->value[this_stack->entry_count];
    }
    else /* index falls within overflow segment */ {
        
        // remember value of first entry in overflow list
        this_value = this_stack->overflow->value;
        
        // isolate first entry in overflow list
        this_entry = this_stack->overflow;
        this_stack->overflow = this_stack->overflow->next;
        
        // remove the entry
        DEALLOCATE(this_entry);
        
        // pass retrieved value and status to caller
        ASSIGN_BY_REF(status, LIFO_STATUS_SUCCESS);
        return this_value;
    } // end if
    
    #undef this_stack
} // end lifo_pop


// ---------------------------------------------------------------------------
// function:  lifo_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns the current capacity of <stack>.  The current capacity is the total
// number of allocated entries. Returns zero if NULL is passed in for <stack>.

lifo_size_t lifo_stack_size(lifo_t stack) {
    #define this_stack ((lifo_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    if (this_stack->entry_count < this_stack->array_size)
        return this_stack->array_size;
    else
        return this_stack->entry_count;
    
    #undef this_stack
} // end lifo_stack_size


// ---------------------------------------------------------------------------
// function:  lifo_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the  number of entries  stored in stack <stack>,  returns  zero if
// NULL is passed in for <stack>.

lifo_size_t lifo_number_of_entries(lifo_t stack) {
    #define this_stack ((lifo_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    return this_stack->entry_count;
    
    #undef this_stack
} // end lifo_number_of_entries


// ---------------------------------------------------------------------------
// function:  lifo_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of LIFO stack object <stack>.  Returns NULL.

lifo_t lifo_dispose_stack(lifo_t stack) {
    #define this_stack ((lifo_s *)stack)
    lifo_entry_s *this_entry;

    // bail out if stack is NULL
    if (stack == NULL)
        return NULL;

    // deallocate any entries in stack's overflow list
    while (this_stack->overflow != NULL) {
        
        // isolate first entry in overflow list
        this_entry = this_stack->overflow;
        this_stack->overflow = this_stack->overflow->next;
        
        // deallocate the entry
        DEALLOCATE(this_entry);
    } // end while
    
    // deallocate stack object and pass NULL to caller
    DEALLOCATE(stack);
    return NULL;
    
    #undef this_stack
} // end lifo_dispose_stack


// END OF FILE
