/* LIFO Storage Library
 *
 *  @file static_lifo.c
 *  Static LIFO implementation
 *
 *  Universal Static Stack
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


#include "static_lifo.h"
#include "../common/alloc.h"
#include "../common/common.h"


// ---------------------------------------------------------------------------
// LIFO stack type
// ---------------------------------------------------------------------------

typedef struct /* static_lifo_s */ {
              cardinal size;
              cardinal entry_count;
    static_lifo_data_t value[0];
} static_lifo_s;


// ---------------------------------------------------------------------------
// function:  static_lifo_new_stack( size, status )
// ---------------------------------------------------------------------------
//
// Creates  and  returns a  new LIFO stack object  with <size> number of value
// slots.  If  zero  is passed in <size>,  then  the new stack will be created
// with the default stack size  as defined by  STATIC_LIFO_DEFAULT_STACK_SIZE.
// Returns NULL if the LIFO object could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

static_lifo_t static_lifo_new_stack(cardinal size,
                        static_lifo_status_t *status) {
    static_lifo_s *stack;
    
    if (size == 0) {
        size = STATIC_LIFO_DEFAULT_STACK_SIZE;
    } // end if
    
    // allocate new stack
    stack = ALLOCATE(sizeof(static_lifo_s) +
                     size * sizeof(static_lifo_data_t));
    
    // bail out if allocation failed
    if (stack == NULL) {
        ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise size and stack entry_count
    stack->size = size;
    stack->entry_count = 0;
    
    // pass status and stack to caller
    ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_SUCCESS);
    return (static_lifo_t) stack;
} // end static_lifo_new_stack


// ---------------------------------------------------------------------------
// function:  static_lifo_push( stack, value, status )
// ---------------------------------------------------------------------------
//
// Adds a  new entry <value>  to the top of stack <stack>.  The  new entry  is
// added by reference,  no data is copied.  However,  no entry is added if the
// the stack is full.  The operation fails if <stack> and/or <value>  is NULL.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void static_lifo_push(static_lifo_t stack,
                 static_lifo_data_t value,
               static_lifo_status_t *status) {
    
    #define this_stack ((static_lifo_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_INVALID_STACK);
        return;
    } // end if
    
    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_INVALID_DATA);
        return;
    } // end if
    
    // bail out if stack is full
    if (this_stack->entry_count >= this_stack->size) {
        ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_STACK_OVERFLOW);
        return;
    } // end if
    
    this_stack->value[this_stack->entry_count] = value;
    this_stack->entry_count++;
    
    ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_SUCCESS);
    return;

    #undef this_stack
} // end static_lifo_push


// ---------------------------------------------------------------------------
// function:  static_lifo_pop( stack, status )
// ---------------------------------------------------------------------------
//
// Removes the top most value from stack <stack> and returns it.  If the stack
// is empty,  then NULL is returned.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

static_lifo_data_t static_lifo_pop(static_lifo_t stack,
                            static_lifo_status_t *status) {
    
    #define this_stack ((static_lifo_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_INVALID_STACK);
        return NULL;
    } // end if
    
    if (this_stack->entry_count == 0) {
        ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_STACK_EMPTY);
        return NULL;
    } // end if
    
    this_stack->entry_count--;
    ASSIGN_BY_REF(status, STATIC_LIFO_STATUS_SUCCESS);
    return this_stack->value[this_stack->entry_count];

    #undef this_stack
} // end static_lifo_pop


// ---------------------------------------------------------------------------
// function:  static_lifo_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns  the number of value slots  of stack <stack>,  returns zero if NULL
// is passed in for <stack>.

cardinal static_lifo_stack_size(static_lifo_t stack) {
    #define this_stack ((static_lifo_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    return this_stack->size;
    
    #undef this_stack
} // end static_lifo_stack_size


// ---------------------------------------------------------------------------
// function:  static_lifo_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the  number of entries  stored in stack <stack>,  returns  zero if
// if NULL is passed in for <stack>.

cardinal static_lifo_number_of_entries(static_lifo_t stack) {
    #define this_stack ((static_lifo_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    return this_stack->entry_count;
    
    #undef this_stack
} // end static_lifo_number_of_entries


// ---------------------------------------------------------------------------
// function:  static_lifo_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of LIFO stack object <stack>.

void static_lifo_dispose_stack(static_lifo_t stack) {
    
    DEALLOCATE(stack);
    stack = NULL;
    
    return;
} // end static_lifo_dispose_stack


// END OF FILE
