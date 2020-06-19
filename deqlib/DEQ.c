/* Double Ended Queue Storage Library
 *
 *  @file DEQ.c
 *  DEQ implementation
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


#include "DEQ.h"
#include "../common/alloc.h"


// ---------------------------------------------------------------------------
// DEQ queue entry pointer type for self referencing declaration of entry type
// ---------------------------------------------------------------------------

struct _deq_entry_s; /* FORWARD */

typedef struct _deq_entry_s *deq_entry_p;


// ---------------------------------------------------------------------------
// DEQ queue entry type
// ---------------------------------------------------------------------------

struct _deq_entry_s {
    deq_data_t value;
    deq_entry_p prev;
    deq_entry_p next;
};

typedef struct _deq_entry_s deq_entry_s;


// ---------------------------------------------------------------------------
// DEQ queue type
// ---------------------------------------------------------------------------

typedef struct /**/ {
       cardinal entry_count;
    deq_entry_p head;
    deq_entry_p tail;
} deq_queue_s;


// ---------------------------------------------------------------------------
// function:  deq_new_queue( status )
// ---------------------------------------------------------------------------
//
// Creates and returns a new queue object.  Returns  NULL  if the queue object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

deq_queue_t deq_new_queue(deq_status_t *status) {
    
    deq_queue_s *new_queue;
    
    new_queue = ALLOCATE(sizeof(deq_queue_s));
    
    // bail out if allocation failed
    if (new_queue == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise
    new_queue->entry_count = 0;
    new_queue->head = NULL;
    new_queue->tail = NULL;
    
    ASSIGN_BY_REF(status, DEQ_STATUS_SUCCESS);
    return (deq_queue_t) new_queue;
} // end deq_new_queue


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
                        deq_status_t *status) {
    
    deq_queue_s *this_queue = (deq_queue_s *) queue;
    deq_entry_s *new_entry;

    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if
    
    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_INVALID_DATA);
        return NULL;
    } // end if
    
    new_entry = ALLOCATE(sizeof(deq_entry_s));
    
    // bail out if allocation failed
    if (new_entry == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise the new entry
    new_entry->value = value;
    new_entry->prev = NULL;
    
    // check if queue is empty
    if (this_queue->entry_count == 0) {
        new_entry->next = NULL;
        this_queue->head = new_entry;
        this_queue->tail = new_entry;
    }
    else /* not empty */ {
        new_entry->next = this_queue->head;
        if (this_queue->head != NULL)
            this_queue->head->prev = new_entry;
        this_queue->head = new_entry;
    } // end if
    
    // update entry counter
    this_queue->entry_count++;
    
    // return queue and status to caller
    ASSIGN_BY_REF(status, DEQ_STATUS_SUCCESS);
    return (deq_queue_t) this_queue;
} // end deq_prepend


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
                        deq_status_t *status) {
    
    deq_queue_s *this_queue = (deq_queue_s *) queue;
    deq_entry_s *new_entry;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if
    
    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_INVALID_DATA);
        return NULL;
    } // end if
    
    new_entry = ALLOCATE(sizeof(deq_entry_s));
    
    // bail out if allocation failed
    if (new_entry == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise the new entry
    new_entry->value = value;
    new_entry->next = NULL;
    
    // check if queue is empty
    if (this_queue->entry_count == 0) {
        new_entry->prev = NULL;
        this_queue->head = new_entry;
        this_queue->tail = new_entry;
    }
    else /* not empty */ {
        new_entry->prev = this_queue->tail;
        if (this_queue->tail != NULL)
            this_queue->tail->next = new_entry;
        this_queue->tail = new_entry;
    } // end if
    
    // update entry counter
    this_queue->entry_count++;
    
    // return queue and status to caller
    ASSIGN_BY_REF(status, DEQ_STATUS_SUCCESS);
    return (deq_queue_t) this_queue;
} // end deq_append


// ---------------------------------------------------------------------------
// function:  deq_first_entry( queue, status )
// ---------------------------------------------------------------------------
//
// Removes  the first entry  from the head of <queue>  and returns it.  If the
// queue is empty,  then NULL is returned.
//
// The function fails if NULL is passed in for <queue>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

deq_data_t deq_first_entry(deq_queue_t queue, deq_status_t *status) {
    
    deq_queue_s *this_queue = (deq_queue_s *) queue;
    deq_entry_s *this_entry;
    deq_data_t *this_value;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if
    
    // bail out if queue is empty
    if (this_queue->entry_count == 0) {
        ASSIGN_BY_REF(status, DEQ_STATUS_QUEUE_EMPTY);
        return NULL;
    } // end if
    
    // remember first entry and its value
    this_entry = this_queue->head;
    this_value = this_entry->value;
    
    // unlink the entry from the queue
    this_queue->head = this_queue->head->next;
    if (this_queue->head != NULL)
        this_queue->head->prev = NULL;
    
    // update entry counter
    this_queue->entry_count--;
    
    // deallocate the entry
    DEALLOCATE(this_entry);
    
    // return its value and status to caller
    ASSIGN_BY_REF(status, DEQ_STATUS_SUCCESS);
    return this_value;
} // end deq_first_entry


// ---------------------------------------------------------------------------
// function:  deq_last_entry( queue, status )
// ---------------------------------------------------------------------------
//
// Removes  the last entry  from the tail of <queue>  and  returns it.  If the
// queue is empty,  then NULL is returned.
//
// The function fails if NULL is passed in for <queue>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

deq_data_t deq_last_entry(deq_queue_t queue, deq_status_t *status) {
    
    deq_queue_s *this_queue = (deq_queue_s *) queue;
    deq_entry_s *this_entry;
    deq_data_t *this_value;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, DEQ_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if
    
    // bail out if queue is empty
    if (this_queue->entry_count == 0) {
        ASSIGN_BY_REF(status, DEQ_STATUS_QUEUE_EMPTY);
        return NULL;
    } // end if
    
    // remember last entry and its value
    this_entry = this_queue->tail;
    this_value = this_entry->value;
    
    // unlink the entry from the queue
    this_queue->tail = this_queue->tail->prev;
    if (this_queue->tail != NULL)
        this_queue->tail->next = NULL;
    
    // update entry counter
    this_queue->entry_count--;
    
    // deallocate the entry
    DEALLOCATE(this_entry);
    
    // return its value and status to caller
    ASSIGN_BY_REF(status, DEQ_STATUS_SUCCESS);
    return this_value;
} // end deq_last_entry


// ---------------------------------------------------------------------------
// function:  deq_number_of_entries( queue )
// ---------------------------------------------------------------------------
//
// Returns the number of entries stored in <queue>,  returns  zero  if NULL is
// passed in for <queue>.

inline cardinal deq_number_of_entries(deq_queue_t queue) {
    
    deq_queue_s *this_queue = (deq_queue_s *) queue;
    
    // bail out if queue is NULL
    if (queue == NULL)
        return 0;
    
    return this_queue->entry_count;
} // end deq_number_of_entries


// ---------------------------------------------------------------------------
// function:  deq_new_iterator( queue )
// ---------------------------------------------------------------------------
//
// Creates and returns  a new iterator object  for iterating entries  in queue
// <queue>.  Returns NULL if <queue> is NULL or empty.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

deq_iterator_t *deq_new_iterator(deq_queue_t queue, deq_status_t *status) {
    
    // TO DO
    
    return NULL;
} // end deq_new_iterator


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

deq_data_t *deq_iterate_next(deq_iterator_t iterator, deq_status_t *status) {
    
    // TO DO
    
    return NULL;
} // end deq_iterate_next


// ---------------------------------------------------------------------------
// function:  deq_dispose_iterator( iterator )
// ---------------------------------------------------------------------------
//
// Disposes of iterator object <iterator>.  Returns NULL.

deq_iterator_t *deq_dispose_iterator(deq_iterator_t iterator) {
    
    // TO DO
    
    return NULL;
} // end deq_dispose_iterator


// ---------------------------------------------------------------------------
// function:  deq_dispose_queue( queue )
// ---------------------------------------------------------------------------
//
// Disposes of queue object <queue>.  Returns NULL.

deq_queue_t *deq_dispose_queue(deq_queue_t queue) {
    
    deq_queue_s *this_queue = (deq_queue_s *) queue;
    deq_entry_s *this_entry;

    if (queue != NULL) {
        
        // deallocate all entries
        while (this_queue->head != NULL) {
            this_entry = this_queue->head;
            this_queue->head = this_entry->next;
            DEALLOCATE(this_entry);
        } // end while
        
        // deallocate queue
        DEALLOCATE(queue);
    } // end if
    
    return NULL;
} // end deq_dispose_queue


// END OF FILE
