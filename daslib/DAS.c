/* Dynamic Array Storage Library
 *
 *  @file DAS.c
 *  DAS implementation
 *
 *  Universal Dynamic Array Storage
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


#include "DAS.h"
#include "../common/alloc.h"
#include "../common/common.h"


// ---------------------------------------------------------------------------
// Integrity checks
// ---------------------------------------------------------------------------

#if (DAS_MINIMUM_ARRAY_SIZE < 2)
#error parameter DAS_MINIMUM_ARRAY_SIZE must not be smaller than 2.
#endif

#if (DAS_DEFAULT_ARRAY_SIZE < 2)
#error parameter DAS_DEFAULT_ARRAY_SIZE must not be smaller than 2.
#endif


// ---------------------------------------------------------------------------
// Array entry type
// ---------------------------------------------------------------------------

typedef struct /* das_data_block_s */ {
    cardinal dummy;
    das_data_t value[];
} das_data_block_s;


// ---------------------------------------------------------------------------
// Array type
// ---------------------------------------------------------------------------

typedef struct /* das_array_s */ {
    cardinal data_block_size;
    cardinal data_block_count;
    das_data_block_s *data_block_selector[];
} das_array_s;


// this_array->data_block_selector[bucket]->value[sub_index] = value;


// ---------------------------------------------------------------------------
// function:  das_new_array( initial_size, status )
// ---------------------------------------------------------------------------
//
// Creates and returns a new array object  with an initial storage capacity of
// <initial_size>.  If  zero  is passed in  for <initial_size>,  then  the new
// array object  will  be created  with a capacity of  DAS_DEFAULT_ARRAY_SIZE.
// Returns NULL if the array object could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

das_array_t das_new_array(cardinal initial_size, das_status_t *status) {
    
    das_array_s *new_array;
    das_data_block_s *new_data_block;
    cardinal index;
    
    if (initial_size == 0)
        initial_size = DAS_DEFAULT_ARRAY_SIZE;
    else if (initial_size < DAS_MINIMUM_ARRAY_SIZE)
        initial_size = DAS_MINIMUM_ARRAY_SIZE;
    
    // allocate new array
    new_array = ALLOCATE(sizeof(das_array_s) +
                         initial_size * sizeof(das_data_block_s));
    
    // bail out if allocation failed
    if (new_array == NULL) {
        ASSIGN_BY_REF(status, DAS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // allocate first data block
    new_data_block = ALLOCATE(initial_size * sizeof(das_data_block_s));

    // bail out if allocation failed
    if (new_data_block == NULL) {
        DEALLOCATE(new_array);
        ASSIGN_BY_REF(status, DAS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise array counters
    new_array->data_block_size = initial_size;
    new_array->data_block_count = initial_size;

    // initialise first data block
    index = 0;
    while (index < initial_size) {
        new_data_block->value[index] = NULL;
        index++;
    } // end while

    // link data block to new array
    new_array->data_block_selector[0] = new_data_block;

    // initialise remaining data block selectors
    index = 1;
    while (index < initial_size) {
        new_array->data_block_selector[index] = NULL;
        index++;
    } // end while
    
    // return array and status to caller
    ASSIGN_BY_REF(status, DAS_STATUS_SUCCESS);
    return (das_array_t) new_array;
} // end das_new_array


// ---------------------------------------------------------------------------
// function:  das_store_entry( array, index, value, status )
// ---------------------------------------------------------------------------
//
// Stores <value> into <array> at <index>  and returns <array>.  The new entry
// is added  by reference,  NO data is copied.  If <index> is  out of range of
// the  current size of <array>,  then <array>  will be  enlarged accordingly.
// The function fails if NULL is passed in for <array>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

das_array_t *das_store_entry(das_array_t array,
                                cardinal index,
                              das_data_t value,
                            das_status_t *status) {
    
    das_array_s *this_array = (das_array_s *) array;
    das_array_s *enlarged_array;
    das_data_block_s *new_data_block;
    cardinal selector_index;
    cardinal data_block_index;
    cardinal new_data_block_count;
    
    // bail out if array is NULL
    if (array == NULL) {
        ASSIGN_BY_REF(status, DAS_STATUS_INVALID_ARRAY);
        return NULL;
    } // end if
        
    selector_index = index / this_array->data_block_size;
    
    if (selector_index > this_array->data_block_count) {
        
        // double data block count until index falls in range
        new_data_block_count = this_array->data_block_count;
        while (selector_index >= new_data_block_count) {
            new_data_block_count = new_data_block_count * 2;
        } // end while
        
        // enlarge array to increase number of data block selectors
        enlarged_array = REALLOCATE(this_array,
                            sizeof(das_array_s) +
                            new_data_block_count * sizeof(das_data_block_s));
        
        if (enlarged_array == NULL) {
            ASSIGN_BY_REF(status, DAS_STATUS_ALLOCATION_FAILED);
            return array;
        } // end if
        
        this_array = enlarged_array;

        // initialise new data block selectors
        index = this_array->data_block_count;
        while (index < new_data_block_count) {
            this_array->data_block_selector[index] = NULL;
        } // end while
        
        this_array->data_block_count = new_data_block_count;
    } // end if
    
    if (this_array->data_block_selector[selector_index] = NULL) {
        
        // allocate new data block
        new_data_block = ALLOCATE(
                    this_array->data_block_size * sizeof(das_data_block_s));
        
        // bail out if allocation failed
        if (new_data_block == NULL) {
            ASSIGN_BY_REF(status, DAS_STATUS_ALLOCATION_FAILED);
            return array;
        } // end if
        
        // initialise new data block
        index = 0;
        while (index < this_array->data_block_size) {
            new_data_block->value[index] = NULL;
            index++;
        } // end while
        
        // link data block to array
        this_array->data_block_selector[selector_index] = new_data_block;

    } // end if
    
    data_block_index = index % this_array->data_block_size;
    
    this_array->data_block_selector[selector_index]->value[data_block_index] =
        value;
    
    // return array and status to caller
    ASSIGN_BY_REF(status, DAS_STATUS_SUCCESS);
    return (das_array_t) this_array;
} // end das_store_entry


// ---------------------------------------------------------------------------
// function:  das_entry_at_index( array, index, status )
// ---------------------------------------------------------------------------
//
// Returns the value stored in <array> at <index>.  If <index> is out of range
// of the current size of <array>,  then NULL is returned.
//
// The function fails if NULL is passed in for <array>.  The status of the op-
// eration is passed back in <status>, unless NULL was passed in for <status>.

das_data_t das_entry_at_index(das_array_t array,
                              cardinal index,
                              das_status_t *status) {
    
    das_array_s *this_array = (das_array_s *) array;
    cardinal selector_index;
    cardinal data_block_index;
    
    // bail out if array is NULL
    if (array == NULL) {
        ASSIGN_BY_REF(status, DAS_STATUS_INVALID_ARRAY);
        return NULL;
    } // end if
        
    selector_index = index / this_array->data_block_size;
    
    if (selector_index > this_array->data_block_count) {
        ASSIGN_BY_REF(status, DAS_STATUS_INVALID_INDEX);
        return NULL;
    } // end if
    
    data_block_index = index % this_array->data_block_size;
    
    // return entry and status to caller;
    ASSIGN_BY_REF(status, DAS_STATUS_SUCCESS);
    return
     this_array->data_block_selector[selector_index]->value[data_block_index];
} // end das_entry_at_index


// ---------------------------------------------------------------------------
// function:  das_array_size( array )
// ---------------------------------------------------------------------------
//
// Returns the  current size  of <array>,  returns  zero  if NULL is passed in
// for <array>.

inline cardinal das_array_size(das_array_t array) {
    
    das_array_s *this_array = (das_array_s *) array;
    
    if (array == NULL)
        return 0;
    
    return this_array->data_block_size * this_array->data_block_count;
} // end das_array_size


// ---------------------------------------------------------------------------
// function:  das_dispose_array( array )
// ---------------------------------------------------------------------------
//
// Disposes of array object <array>.  Returns NULL.

das_array_t *das_dispose_array(das_array_t array) {
    
    das_array_s *this_array = (das_array_s *) array;
    cardinal index;
    
    if (array != NULL) {
        
        // deallocate all data blocks
        index = 0;
        while (index < this_array->data_block_count) {
            DEALLOCATE(this_array->data_block_selector[index]);
            index++;
        } // end while
        
        // deallocate array
        DEALLOCATE(array);
    } // end if
    
    return NULL;
} // end das_dispose_array


// END OF FILE
