/* BST Storage Library
 *
 *  @file Splay.h
 *  Splay interface
 *
 *  Universal Splay Tree
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


#ifndef SPLAY_H
#define SPLAY_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Opaque AA tree handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t splay_tree_t;


// ---------------------------------------------------------------------------
// Key type
// ---------------------------------------------------------------------------

typedef void *splay_key_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *splay_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* splay_status_t */ {
    SPLAY_STATUS_SUCCESS = 1,
    SPLAY_STATUS_INVALID_TREE,
    SPLAY_STATUS_INVALID_DATA,
    SPLAY_STATUS_INVALID_KEY,
    SPLAY_STATUS_ENTRY_NOT_FOUND,
    SPLAY_STATUS_KEY_NOT_UNIQUE,
    SPLAY_STATUS_ALLOCATION_FAILED
} splay_status_t;


// ---------------------------------------------------------------------------
// function:  splay_new_tree( status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new tree object.  Returns  NULL  if the tree object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

splay_tree_t splay_new_tree(splay_status_t *status);


// ---------------------------------------------------------------------------
// function:  splay_store_entry( tree, key, value, status )
// ---------------------------------------------------------------------------
//
// Stores <value> for <key>  in <tree>.  The new entry is added  by reference,
// NO data is copied.  The function fails  if NULL is passed in  for <tree> or
// <value> or if zero is passed in for <key>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void splay_store_entry(splay_tree_t tree,
                        splay_key_t key,
                       splay_data_t value,
                     splay_status_t *status);


// ---------------------------------------------------------------------------
// function:  splay_value_for_key( tree, index, status )
// ---------------------------------------------------------------------------
//
// Returns the value stored for <key< in <tree>.  If no value for <key> exists
// in <tree>,  or if NULL is passed in for <tree>,  then NULL is returned.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

splay_data_t splay_value_for_key(splay_tree_t tree,
                                  splay_key_t key,
                               splay_status_t *status);


// ---------------------------------------------------------------------------
// function:  splay_number_of_entries( tree )
// ---------------------------------------------------------------------------
//
// Returns the number of entries  stored in <tree>,  returns  zero  if NULL is
// passed in for <tree>.

inline cardinal splay_number_of_entries(splay_tree_t tree);


// ---------------------------------------------------------------------------
// function:  splay_remove_entry( tree )
// ---------------------------------------------------------------------------
//
// Removes the entry stored for <key> from <tree>.  The function fails if NULL
// is passed in for <tree> or if no entry for <key> is stored in <tree>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void splay_remove_entry(splay_tree_t tree,
                         splay_key_t key,
                      splay_status_t *status);


// ---------------------------------------------------------------------------
// function:  splay_dispose_tree( tree )
// ---------------------------------------------------------------------------
//
// Disposes of tree object <tree>.  Returns NULL.

splay_tree_t *splay_dispose_tree(splay_tree_t tree);


#endif /* SPLAY_H */

// END OF FILE
