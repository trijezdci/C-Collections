/* Dynamic Array Storage Library
 *
 *  @file DAS.h
 *  DAS interface
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


#ifndef DAS_H
#define DAS_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Minimum array size
// ---------------------------------------------------------------------------

#define DAS_MINIMUM_ARRAY_SIZE 8


// ---------------------------------------------------------------------------
// Default array size
// ---------------------------------------------------------------------------

#define DAS_DEFAULT_ARRAY_SIZE 256


// ---------------------------------------------------------------------------
// Opaque DAS array handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t das_array_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *das_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* das_status_t */ {
    DAS_STATUS_SUCCESS = 1,
    DAS_STATUS_INVALID_ARRAY,
    DAS_STATUS_INVALID_DATA,
    DAS_STATUS_INVALID_INDEX,
    DAS_STATUS_ALLOCATION_FAILED
} das_status_t;


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

das_array_t das_new_array(cardinal initial_size, das_status_t *status);


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
                            das_status_t *status);


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
                             das_status_t *status);


// ---------------------------------------------------------------------------
// function:  das_array_size( array )
// ---------------------------------------------------------------------------
//
// Returns the  current size  of <array>,  returns  zero  if NULL is passed in
// for <array>.

inline cardinal das_array_size(das_array_t array);


// ---------------------------------------------------------------------------
// function:  das_dispose_array( array )
// ---------------------------------------------------------------------------
//
// Disposes of array object <array>.  Returns NULL.

das_array_t *das_dispose_array(das_array_t array);


#endif /* DAS_H */

// END OF FILE
