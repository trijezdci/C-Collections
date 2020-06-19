/* FIFO Storage Library
 *
 *  @file fifo_static.c
 *  Static queue implementation
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


#include "../common/alloc.h"
#include "../common/common.h"
#include "fifo_static.h"


// ---------------------------------------------------------------------------
// Static queue type
// ---------------------------------------------------------------------------

typedef struct /* fifo_s */ {
    cardinal size;
    cardinal entry_count;
    cardinal head;
    cardinal tail;
    fifo_data_t value[0];
} fifo_s;


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

fifo_t fifo_new_queue(cardinal size, fifo_status_t *status) {
    
    fifo_s *new_queue;
    
    if (size == 0) {
        size = FIFO_DEFAULT_QUEUE_SIZE;
    } // end if
    
    // allocate new queue
    new_queue = ALLOCATE(sizeof(fifo_s) + size * sizeof(fifo_data_t));
    
    // bail out if allocation failed
    if (new_queue == NULL) {
        ASSIGN_BY_REF(status, FIFO_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise the new queue
    new_queue->size = size;
    new_queue->entry_count = 0;
    new_queue->head = 0;
    new_queue->tail = 0;
    new_queue->value[0] = NULL;
    
    // pass status and queue to caller
    ASSIGN_BY_REF(status, FIFO_STATUS_SUCCESS);
    return (fifo_t) new_queue;
} // end fifo_new_queue


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

fifo_t *fifo_enqueue(fifo_t queue, fifo_data_t value, fifo_status_t *status) {
    
    fifo_s *this_queue = (fifo_s *) queue;

    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, FIFO_STATUS_INVALID_QUEUE);
        return queue;
    } // end if
    
    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, FIFO_STATUS_INVALID_DATA);
        return queue;
    } // end if
    
    // check if queue is full
    if (this_queue->entry_count == this_queue->size) {
        ASSIGN_BY_REF(status, FIFO_STATUS_QUEUE_OVERFLOW);
        return queue;
    } // end if
    
    this_queue->value[this_queue->head] = value;
    this_queue->entry_count++;
    
    this_queue->head++;
    if (this_queue->head >= this_queue->size)
        this_queue->head = 0;

    ASSIGN_BY_REF(status, FIFO_STATUS_SUCCESS);
    return (fifo_t) this_queue;
} // end fifo_enqueue


// ---------------------------------------------------------------------------
// function:  fifo_dequeue( queue, status )
// ---------------------------------------------------------------------------
//
// Removes the oldest value from the tail of queue <queue> and returns it.  If
// the queue is empty,  then NULL is returned.
//
// The function fails if NULL is passed in for <queue>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

fifo_data_t fifo_dequeue(fifo_t queue, fifo_status_t *status) {
    
    fifo_s *this_queue = (fifo_s *) queue;
    cardinal oldest;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, FIFO_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if
    
    if (this_queue->entry_count == 0) {
        ASSIGN_BY_REF(status, FIFO_STATUS_QUEUE_EMPTY);
        return NULL;
    } // end if
    
    oldest = this_queue->tail;
    this_queue->entry_count--;
    
    this_queue->tail++;
    if (this_queue->tail >= this_queue->size)
        this_queue->tail = 0;
    
    ASSIGN_BY_REF(status, FIFO_STATUS_SUCCESS);
    return this_queue->value[oldest];
} // end fifo_dequeue


// ---------------------------------------------------------------------------
// function:  fifo_queue_size( queue )
// ---------------------------------------------------------------------------
//
// Returns the total capacity of <queue>,  returns  zero  if NULL is passed in
// for <queue>.

cardinal fifo_queue_size(fifo_t queue) {
    
    fifo_s *this_queue = (fifo_s *) queue;
    
    // bail out if queue is NULL
    if (queue == NULL)
        return 0;
   
    return this_queue->size;
} // end fifo_queue_size


// ---------------------------------------------------------------------------
// function:  fifo_number_of_entries( queue )
// ---------------------------------------------------------------------------
//
// Returns the number of entries stored in <queue>,  returns  zero  if NULL is
// passed in for <queue>.

cardinal fifo_number_of_entries(fifo_t queue) {
    
    fifo_s *this_queue = (fifo_s *) queue;
    
    // bail out if queue is NULL
    if (queue == NULL)
        return 0;
    
    return this_queue->entry_count;
} // end fifo_number_of_entries


// ---------------------------------------------------------------------------
// function:  fifo_queue_is_resizable( queue )
// ---------------------------------------------------------------------------
//
// Returns false.

bool fifo_queue_is_resizable(fifo_t queue) {
    return false;
} // end fifo_queue_is_resizable


// ---------------------------------------------------------------------------
// function:  fifo_dispose_queue( queue )
// ---------------------------------------------------------------------------
//
// Disposes of queue object <queue>.  Returns NULL.

fifo_t *fifo_dispose_queue(fifo_t queue) {
    
    DEALLOCATE(queue);
    return NULL;
} // end fifo_dispose_queue


// END OF FILE
