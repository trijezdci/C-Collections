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


#include "PQ.h"
#include "../common/alloc.h"


// ---------------------------------------------------------------------------
// Integrity checks
// ---------------------------------------------------------------------------

#if (PQ_MINIMUM_CAPACITY < 5)
#error PQ_MINIMUM_CAPACITY must not be less than 5
#elif (PQ_MINIMUM_CAPACITY > 255)
#warning PQ_MINIMUM_CAPACITY is unreasonably high, recommended value is 255
#endif

#if (PQ_MAXIMUM_CAPACITY < PQ_MINIMUM_CAPACITY)
#error PQ_MAXIMUM_CAPACITY must not be less than PQ_MINIMUM_CAPACITY
#elif (PQ_MAXIMUM_CAPACITY >= UINT64_MAX)
#error PQ_MAXIMUM_CAPACITY must not exceed range of uint64_t
#endif


// ---------------------------------------------------------------------------
// Binomial queue node pointer type for self referencing declaration of node
// ---------------------------------------------------------------------------

struct _bq_node_s; /* FORWARD */

typedef struct _bq_node_s *bq_node_p;


// ---------------------------------------------------------------------------
// Binomial queue node type
// ---------------------------------------------------------------------------

struct _bq_node_s {
    pq_data_t value;
    bq_node_p left;
    bq_node_p right;
};

typedef struct _bq_node_s bq_node_s;


// ---------------------------------------------------------------------------
// PQ queue type
// ---------------------------------------------------------------------------

typedef struct /* pq_s */ {
    pq_counter_t capacity;
    pq_counter_t entry_count;
    pq_compare_f has_priority;
    uint_fast8_t heap_size;
    uint_fast8_t heap_count;
       bq_node_p heap[];
} pq_s;


fmacro uint_fast8_t _heap_size_for_capacity(pq_counter_t capacity);


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
                   pq_status_t *status) {
    
    pq_s *new_queue;
    cardinal index, heap_size;
    
    // bail out if compare_function is NULL
    if (compare_function == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_INVALID_COMPARE_FUNCTION);
        return NULL;
    } // end if
    
    // check capacity
    if (capacity == 0) {
        capacity = PQ_MAXIMUM_CAPACITY;
    }
    else if (capacity < PQ_MINIMUM_CAPACITY) {
        capacity = PQ_MINIMUM_CAPACITY;
    }
    else if (capacity > PQ_MAXIMUM_CAPACITY) {
        ASSIGN_BY_REF(status, PQ_STATUS_INVALID_CAPACITY);
        return NULL;
    } // end if
    
    // calculate heap size
    heap_size = _heap_size_for_capacity(capacity);
    
    // allocate new queue
    new_queue = ALLOCATE(sizeof(pq_s) + heap_size * sizeof(bq_node_p));
    
    // bail out if allocation failed
    if (new_queue == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
        
    // initialise new queue
    new_queue->capacity = capacity;
    new_queue->entry_count = 0;
    new_queue->has_priority = compare_function;
    new_queue->heap_size = heap_size;
    new_queue->heap_count = 0;

    // initialise heap pointers
    index = 0;
    while (index < heap_size) {
        new_queue->heap[index] = NULL;
        index++;
    } // end while
    
    // pass queue and status back to caller
    ASSIGN_BY_REF(status, PQ_STATUS_SUCCESS);
    return (pq_t) new_queue;
} // end pq_new_queue


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

void pq_enqueue(pq_t queue, pq_data_t value, pq_status_t *status) {
    
    pq_s *this_queue = (pq_s *) queue;
    bq_node_s *new_node, *carry_node;
    cardinal index;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_INVALID_QUEUE);
        return;
    } // end if

    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_INVALID_DATA);
        return;
    } // end if

    // bail out if capacity is insufficient
    if (this_queue->entry_count >= this_queue->capacity) {
        ASSIGN_BY_REF(status, PQ_STATUS_QUEUE_OVERLOW);
        return;
    } // end if
    
    // allocate a new node
    new_node = ALLOCATE(sizeof(bq_node_s));
    
    // bail out if allocation failed
    if (new_node == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_ALLOCATION_FAILED);
        return;
    } // end if
    
    // initialise new node
    new_node->value = value;
    new_node->left = NULL;
    new_node->right = NULL;
    
    // insert new node
    index = 0;
    carry_node = new_node;
    while ((carry_node != NULL) && (index < this_queue->heap_size)) {
                
        // place carry node in root node at index if empty
        if (this_queue->heap[index] == NULL) {
            this_queue->heap[index] = carry_node;
            break;
        } // end if
        
        if /* carry node has priority */ (this_queue->has_priority(
            carry_node->value, this_queue->heap[index]->value)) {
                
            this_queue->heap[index]->right = carry_node->left;
            carry_node->left = this_queue->heap[index];
        }
        else  /* carry node does not have priority */ {
            carry_node->right = this_queue->heap[index]->left;
            this_queue->heap[index]->left = carry_node;
            carry_node = this_queue->heap[index];
        } // end if
            
        this_queue->heap[index] = NULL;
        this_queue->heap_count = index + 1;
    } // end while
    
    // update entry counter
    this_queue->entry_count++;
    
    // pass status back to caller
    ASSIGN_BY_REF(status, PQ_STATUS_SUCCESS);
    return;
} // end pq_enqueue


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

pq_data_t pq_inspect_next(pq_t queue, pq_status_t *status) {
    
    pq_s *this_queue = (pq_s *) queue;
    cardinal index, high_prio_index;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if
    
    // bail out if queue is empty
    if (this_queue->entry_count == 0) {
        ASSIGN_BY_REF(status, PQ_STATUS_QUEUE_EMPTY);
        return NULL;
    } // end if
    
    // find first non-empty heap
    index = 0;
    while ((this_queue->heap[index] == NULL) &&
           (index < this_queue->heap_count))
        index++;
    
    // remember this index
    high_prio_index = index;
    index++;
    
    // find heap with highest priority root
    while (index < this_queue->heap_count) {
        if (this_queue->heap[index] != NULL) {
            if (this_queue->has_priority(this_queue->heap[index],
                                         this_queue->heap[high_prio_index])) {
                high_prio_index = index;
            } // end if
        } // end if
        index++;
    } // end while
    
    return (pq_data_t) this_queue->heap[high_prio_index]->value;
} // end pq_inspect_next


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

pq_data_t pq_dequeue(pq_t queue, pq_status_t *status) {
    
    pq_s *this_queue = (pq_s *) queue;
    cardinal index, high_prio_index;
    pq_data_t this_entry = NULL;
    
    // bail out if queue is NULL
    if (queue == NULL) {
        ASSIGN_BY_REF(status, PQ_STATUS_INVALID_QUEUE);
        return NULL;
    } // end if

    // bail out if queue is empty
    if (this_queue->entry_count == 0) {
        ASSIGN_BY_REF(status, PQ_STATUS_QUEUE_EMPTY);
        return NULL;
    } // end if
    
    // find first non-empty heap
    index = 0;
    while ((this_queue->heap[index] == NULL) &&
           (index < this_queue->heap_count))
        index++;
    
    // remember this index
    high_prio_index = index;
    index++;
    
    // find heap with highest priority root
    while (index < this_queue->heap_count) {
        if (this_queue->heap[index] != NULL) {
            if (this_queue->has_priority(this_queue->heap[index],
                                         this_queue->heap[high_prio_index])) {
                high_prio_index = index;
            } // end if
        } // end if
        index++;
    } // end while
    
    // TO DO
    
    // remove node containing highest priority entry from its heap
    
    // get its value and deallocate the node
    
    // make binomial queue from remaining nodes
    
    // merge resulting queue back in
    
    // adjust counters
    this_queue->entry_count--;
    
    return this_entry;
} // end pq_dequeue


// ---------------------------------------------------------------------------
// function:  pq_number_of_entries( queue )
// ---------------------------------------------------------------------------
//
// Returns the number of entries stored in <queue>,  returns  zero  if NULL is
// passed in for <queue>.

inline cardinal pq_number_of_entries(pq_t queue) {

    pq_s *this_queue = (pq_s *) queue;
    
    if (queue == NULL)
        return 0;

    return this_queue->entry_count;
} // end pq_number_of_entries


// ---------------------------------------------------------------------------
// function:  pq_dispose_stack( queue )
// ---------------------------------------------------------------------------
//
// Disposes of queue object <queue>.  Returns NULL.

pq_t *pq_dispose_queue(pq_t queue) {

    pq_s *this_queue = (pq_s *) queue;

    if (queue != NULL) {
        // deallocate all nodes
        
        // TO DO
        
        // deallocate the queue
        DEALLOCATE(this_queue);
    } // end if

    return NULL;
} // end pq_dispose_queue


// ---------------------------------------------------------------------------
// provate function:  _root_node_count_for_capacity( capacity )
// ---------------------------------------------------------------------------
//
// Returns the number of heaps required for a queue capacity of <capacity>.

fmacro uint_fast8_t _heap_size_for_capacity(pq_counter_t capacity) {
    
    pq_counter_t limit = 1;
    uint_fast8_t heap_size = 1;
    
    while (capacity >= limit) {
        limit = limit << 1;
        heap_size++;
    } // end while
    
    return heap_size;
} // end _root_node_count_for_capacity


// END OF FILE
