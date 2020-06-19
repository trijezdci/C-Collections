/* FIFO Storage Library
 *
 *  @file fifo_static.h
 *  Static queue interface
 *
 *  Universal Static Queue
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


#ifndef FIFO_STATIC_H
#define FIFO_STATIC_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Default queue size
// ---------------------------------------------------------------------------

#define FIFO_DEFAULT_QUEUE_SIZE 256


// ---------------------------------------------------------------------------
// Opaque FIFO handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t fifo_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *fifo_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* fifo_status_t */ {
    FIFO_STATUS_SUCCESS = 1,
    FIFO_STATUS_INVALID_QUEUE,
    FIFO_STATUS_INVALID_DATA,
    FIFO_STATUS_QUEUE_OVERFLOW,
    FIFO_STATUS_QUEUE_EMPTY,
    FIFO_ALLOCATION_FAILED
} fifo_status_t;


// ---------------------------------------------------------------------------
// function:  fifo_new_queue( size, status )
// ---------------------------------------------------------------------------
//
// Creates and returns  a new queue object  with a storage capacity of <size>.
// If zero is passed in for <size>,  then the new queue object will be created
// with a capacity of  FIFO_DEFAULT_QUEUE_SIZE.  Returns  NULL  if  the  queue
// object could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

fifo_t fifo_new_queue(cardinal size, fifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  fifo_enqueue( queue, value, status )
// ---------------------------------------------------------------------------
//
// Adds a new entry <value> to the head of queue <queue>  and returns <queue>.
// The new entry  is added  by reference,  NO data is copied.  If the queue is
// full,  then  NO  new entry is added  to <queue>  and NULL is returned.  The
// function fails if NULL is passed in for <queue> or <value>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

fifo_t *fifo_enqueue(fifo_t queue, fifo_data_t value, fifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  fifo_dequeue( queue, status )
// ---------------------------------------------------------------------------
//
// Removes the oldest value from the tail of queue <queue> and returns it.  If
// the queue is empty,  then NULL is returned.
//
// The function fails if NULL is passed in for <queue>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

fifo_data_t fifo_dequeue(fifo_t queue, fifo_status_t *status);


// ---------------------------------------------------------------------------
// function:  fifo_queue_size( queue )
// ---------------------------------------------------------------------------
//
// Returns the total capacity of <queue>,  returns  zero  if NULL is passed in
// for <queue>.

cardinal fifo_queue_size(fifo_t queue);


// ---------------------------------------------------------------------------
// function:  fifo_number_of_entries( queue )
// ---------------------------------------------------------------------------
//
// Returns the number of entries stored in <queue>,  returns  zero  if NULL is
// passed in for <queue>.

cardinal fifo_number_of_entries(fifo_t queue);


// ---------------------------------------------------------------------------
// function:  fifo_queue_is_resizable( queue )
// ---------------------------------------------------------------------------
//
// Returns false.

bool fifo_queue_is_resizable(fifo_t queue);


// ---------------------------------------------------------------------------
// function:  fifo_dispose_queue( queue )
// ---------------------------------------------------------------------------
//
// Disposes of queue object <queue>.  Returns NULL.

fifo_t *fifo_dispose_queue(fifo_t queue);


#endif /* FIFO_STATIC_H */

// END OF FILE
