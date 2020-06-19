/* Double Ended Queue Storage Library
 *
 *  @file DEQ.h
 *  DEQ interface
 *
 *  Universal Double Ended Queue
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


#ifndef DEQ_H
#define DEQ_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Opaque DEQ handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t deq_queue_t;


// ---------------------------------------------------------------------------
// Opaque DEQ iterator handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t deq_iterator_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *deq_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* deq_status_t */ {
    DEQ_STATUS_SUCCESS = 1,
    DEQ_STATUS_INVALID_QUEUE,
    DEQ_STATUS_INVALID_DATA,
    DEQ_STATUS_QUEUE_EMPTY,
    DEQ_STATUS_ALLOCATION_FAILED,
} deq_status_t;


// ---------------------------------------------------------------------------
// function:  deq_new_queue( status )
// ---------------------------------------------------------------------------
//
// Creates and returns a new queue object.  Returns  NULL  if the queue object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

deq_queue_t deq_new_queue(deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_prepend( queue, value, status )
// ---------------------------------------------------------------------------
//
// Prepends a new entry <value>  at the head of <queue>  and returns a pointer
// to the queue.  The new entry is added  by reference,  no data is copied.
//
// If  NULL  is passed in for <queue> or <value>,  then the function fails and
// returns  the value passed in for <queue>.  The status  of the operation  is
// passed back in <status>,  unless NULL was passed in for <status>.

deq_queue_t *deq_prepend(deq_queue_t queue,
                          deq_data_t value,
                        deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_append( queue, value, status )
// ---------------------------------------------------------------------------
//
// Appends a new entry <value> to the tail of <queue> and returns a pointer to
// the queue.  The new entry is added  by reference,  no data is copied.
//
// If  NULL  is passed in for <queue> or <value>,  then the function fails and
// returns  the value passed in for <queue>.  The status  of the operation  is
// passed back in <status>,  unless NULL was passed in for <status>.

deq_queue_t *deq_append(deq_queue_t queue,
                         deq_data_t value,
                       deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_first_entry( queue, status )
// ---------------------------------------------------------------------------
//
// Removes  the first entry  from the head of <queue>  and returns it.  If the
// queue is empty,  then NULL is returned.
//
// The function fails if NULL is passed in for <queue>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

deq_data_t deq_first_entry(deq_queue_t queue, deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_last_entry( queue, status )
// ---------------------------------------------------------------------------
//
// Removes  the last entry  from the tail of <queue>  and  returns it.  If the
// queue is empty,  then NULL is returned.
//
// The function fails if NULL is passed in for <queue>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

deq_data_t deq_last_entry(deq_queue_t queue, deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_number_of_entries( queue )
// ---------------------------------------------------------------------------
//
// Returns the number of entries stored in <queue>,  returns  zero  if NULL is
// passed in for <queue>.

inline cardinal deq_number_of_entries(deq_queue_t queue);


// ---------------------------------------------------------------------------
// function:  deq_new_iterator( queue )
// ---------------------------------------------------------------------------
//
// Creates and returns  a new iterator object  for iterating entries  in queue
// <queue>.  Returns NULL if <queue> is NULL or empty.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

deq_iterator_t *deq_new_iterator(deq_queue_t queue, deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_iterate_next( iterator )
// ---------------------------------------------------------------------------
//
// The  first  call  to this function  returns the  first  entry  of the queue
// associated with <iterator>.  Subsequent calls return the queue's respective
// successor entries.  Returns NULL if  any preceeding call  returned the last
// entry of the queue.
//
// If  NULL  is passed in for <iterator>,  then the function fails and returns
// NULL.  The status of the operation is passed back in <status>,  unless NULL
// was passed in for <status>.

deq_data_t *deq_iterate_next(deq_iterator_t iterator, deq_status_t *status);


// ---------------------------------------------------------------------------
// function:  deq_dispose_iterator( iterator )
// ---------------------------------------------------------------------------
//
// Disposes of iterator object <iterator>.  Returns NULL.

deq_iterator_t *deq_dispose_iterator(deq_iterator_t iterator);


// ---------------------------------------------------------------------------
// function:  deq_dispose_queue( queue )
// ---------------------------------------------------------------------------
//
// Disposes of queue object <queue>.  Returns NULL.

deq_queue_t *deq_dispose_queue(deq_queue_t queue);


#endif /* DEQ_H */

// END OF FILE
