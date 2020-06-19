/* Priority Queue Library
 *
 *  @file PQ.h
 *  PQ interface
 *
 *  Universal Priority Queue
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


#ifndef PQ_H
#define PQ_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Minimum queue capacity
// ---------------------------------------------------------------------------

#define PQ_MINIMUM_CAPACITY 255  /* 255 objects */


// ---------------------------------------------------------------------------
// Maximum queue capacity
// ---------------------------------------------------------------------------

#define PQ_MAXIMUM_CAPACITY (16*1024*1024 - 1)  /* 16.777.215 objects */


// ---------------------------------------------------------------------------
// Determine the size of integers needed to encode maximum capacity
// ---------------------------------------------------------------------------

#if (PQ_MAXIMUM_CAPACITY >= (1 << 32))
#define PQ_REQUIRED_UINT uint64_t
#elif (PQ_MAXIMUM_CAPACITY >= (1 << 16))
#define PQ_REQUIRED_UINT uint32_t
#elif /* max capacity fits in 16 bit */
#define PQ_REQUIRED_UINT uint16_t
#endif


// ---------------------------------------------------------------------------
// Opaque PQ handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t pq_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *pq_data_t;


// ---------------------------------------------------------------------------
// Entry count type
// ---------------------------------------------------------------------------

typedef PQ_REQUIRED_UINT pq_counter_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* pq_status_t */ {
    PQ_STATUS_SUCCESS = 1,
    PQ_STATUS_INVALID_QUEUE,
    PQ_STATUS_INVALID_DATA,
    PQ_STATUS_INVALID_CAPACITY,
    PQ_STATUS_INVALID_COMPARE_FUNCTION,
    PQ_STATUS_QUEUE_EMPTY,
    PQ_STATUS_QUEUE_OVERLOW,
    PQ_STATUS_ALLOCATION_FAILED
} pq_status_t;


// ---------------------------------------------------------------------------
// Priority comparison callback function type
// ---------------------------------------------------------------------------

typedef bool (*pq_compare_f)(pq_data_t, pq_data_t);


// ---------------------------------------------------------------------------
// function:  pq_new_queue( capacity, compare_function, status )
// ---------------------------------------------------------------------------
//
// Creates and returns a new queue object with storage capacity of <capacity>.
// If zero is passed in for <capacity>,  then the queue will be created with a
// capacity of  PQ_MAXIMUM_CAPACITY.  If a value  greater than zero  but  less
// than PQ_MINIMUM_CAPACITY is passed in for <capacity>,  then  the queue will
// queue will be created  with a capacity of  PQ_MINIMUM_CAPACITY.  If a value
// greater than PQ_MAXIMUM_CAPACITY is passed in for <capacity>,  or if memory
// allocation fails,  then the function fails and returns NULL.
//
// The  capacity of a queue  is the number of entries it can hold.  The under-
// lying implementation uses a binomial queue (a set of power-of-two heaps) as
// its internal storage structure.  Whilst storage for entries  are  allocated
// dynamically,  pointers for the  power-of-two heaps  are allocated  when the
// queue is created.  The number of heap pointers is determined by the queue's
// capacity using the formula:  heap count = log2 ( capacity + 1).
//
// The  priority  of entries to be added to the queue is determined by calling
// the  comparison  callback  function  passed in for <compare_function>.  The
// function must accept two arguments of type pq_data_t and return true if the
// first argument has higher priority than the second argument.
//
// If  NULL  is passed in  for <has_priority>,  then  the function fails.  The
// status of the operation is passed back in <status>,  unless NULL was passed
// in for <status>.

pq_t pq_new_queue(pq_counter_t capacity,
                  pq_compare_f compare_function,
                   pq_status_t *status);


// ---------------------------------------------------------------------------
// function:  pq_enqueue( queue, value, status )
// ---------------------------------------------------------------------------
//
// Adds a new entry <value> to priority queue <queue>.  The new entry is added
// by reference, no data is copied.  No entry is added if the queue's capacity
// is insufficient to hold the new entry,  or if memory allocation fails.
//
// If NULL is passed in for <queue> or <value>,  then the function fails.  The
// status of the operation is passed back in <status>,  unless NULL was passed
// in for <status>.

void pq_enqueue(pq_t queue, pq_data_t value, pq_status_t *status);


// ---------------------------------------------------------------------------
// function:  pq_inspect_next( queue, status )
// ---------------------------------------------------------------------------
//
// Retrieves the entry with the highest priority from <queue>  and  returns it
// without removing the entry.  If the queue is empty,  then NULL is returned.
//
// If  NULL  is passed in for <queue>,  then  the function fails  and  NULL is
// returned.  The status of the operation  is passed back in <status>,  unless
// NULL was passed in for <status>.

pq_data_t pq_inspect_next(pq_t queue, pq_status_t *status);


// ---------------------------------------------------------------------------
// function:  pq_dequeue( queue, status )
// ---------------------------------------------------------------------------
//
// Removes  the entry with the highest priority  from <queue>  and returns it.
// If the queue is empty,  then NULL is returned.
//
// If  NULL  is passed in for <queue>,  then  the function fails  and  NULL is
// returned.  The status of the operation  is passed back in <status>,  unless
// NULL was passed in for <status>.

pq_data_t pq_dequeue(pq_t queue, pq_status_t *status);


// ---------------------------------------------------------------------------
// function:  pq_number_of_entries( queue )
// ---------------------------------------------------------------------------
//
// Returns the number of entries stored in <queue>,  returns  zero  if NULL is
// passed in for <queue>.

inline pq_counter_t pq_number_of_entries(pq_t queue);


// ---------------------------------------------------------------------------
// function:  pq_dispose_stack( queue )
// ---------------------------------------------------------------------------
//
// Disposes of queue object <queue>.  Returns NULL.

pq_t *pq_dispose_queue(pq_t queue);


#endif /* PQ_H */

// END OF FILE
