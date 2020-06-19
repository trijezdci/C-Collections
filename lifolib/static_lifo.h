/* LIFO Storage Library
 *
 *  @file static_lifo.h
 *  Static LIFO interface
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


#ifndef STATIC_LIFO_H
#define STATIC_LIFO_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Default stack size
// ---------------------------------------------------------------------------

#define STATIC_LIFO_DEFAULT_STACK_SIZE 256


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

typedef opaque_t static_lifo_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *static_lifo_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* static_lifo_status_t */ {
    STATIC_LIFO_STATUS_SUCCESS = 1,
    STATIC_LIFO_STATUS_INVALID_STACK,
    STATIC_LIFO_STATUS_INVALID_DATA,
    STATIC_LIFO_STATUS_STACK_OVERFLOW,
    STATIC_LIFO_STATUS_STACK_EMPTY,
    STATIC_LIFO_STATUS_ALLOCATION_FAILED
} static_lifo_status_t;


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
                        static_lifo_status_t *status);


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
               static_lifo_status_t *status);


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
                            static_lifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  static_lifo_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns  the number of value slots  of stack <stack>,  returns zero if NULL
// is passed in for <stack>.

cardinal static_lifo_stack_size(static_lifo_t stack);


// ---------------------------------------------------------------------------
// function:  static_lifo_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the  number of entries  stored in stack <stack>,  returns  zero if
// if NULL is passed in for <stack>.

cardinal static_lifo_number_of_entries(static_lifo_t stack);


// ---------------------------------------------------------------------------
// function:  static_lifo_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of LIFO stack object <stack>.

void static_lifo_dispose_stack(static_lifo_t stack);


#endif /* STATIC_LIFO_H */

// END OF FILE
