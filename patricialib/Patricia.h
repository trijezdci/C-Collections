/* Patricia Trie Library
 *
 *  @file Patricia.h
 *  Patricia interface
 *
 *  Universal Patricia Trie
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


#ifndef PTRIE_H
#define PTRIE_H


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Maximum key length
// ---------------------------------------------------------------------------

#define PTRIE_MAXIMUM_KEY_LENGTH (4 * 1024 - 1)  /* 4095 characters */


// ---------------------------------------------------------------------------
// Maximum number of entries
// ---------------------------------------------------------------------------

#define PTRIE_MAXIMUM_ENTRY_COUNT (0xfffffffe)  /* about 2 billion entries */


// ---------------------------------------------------------------------------
// Determine suitable unsigned integer type for entry counter
// ---------------------------------------------------------------------------

#if (PTRIE_MAXIMUM_ENTRY_COUNT * 8 <= ((1 << 8) - 1))
#define PTRIE_ENTRY_COUNT_BASE_TYPE uint8_t
#elif (PTRIE_MAXIMUM_ENTRY_COUNT * 8 <= ((1 << 16) - 1))
#define PTRIE_ENTRY_COUNT_BASE_TYPE uint16_t
#elif (PTRIE_MAXIMUM_ENTRY_COUNT * 8 <= ((1 << 32) - 1))
#define PTRIE_ENTRY_COUNT_BASE_TYPE uint32_t
#elif (PTRIE_MAXIMUM_ENTRY_COUNT * 8 <= ((1 << 64UL) - 1))
#define PTRIE_ENTRY_COUNT_BASE_TYPE uint64_t
#else /* out of range for 64 bit unsigned */
#error PTRIE_MAXIMUM_ENTRY_COUNT too large
#endif


// ---------------------------------------------------------------------------
// Opaque trie handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t ptrie_t;


// ---------------------------------------------------------------------------
// Key type
// ---------------------------------------------------------------------------

typedef char *ptrie_key_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *ptrie_data_t;


// ---------------------------------------------------------------------------
// Entry counter type
// ---------------------------------------------------------------------------

typedef PTRIE_ENTRY_COUNT_BASE_TYPE ptrie_counter_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* ptrie_status_t */ {
    PTRIE_STATUS_SUCCESS = 1,
    PTRIE_STATUS_INVALID_TRIE,
    PTRIE_STATUS_INVALID_KEY,
    PTRIE_STATUS_INVALID_DATA,
    PTRIE_STATUS_INVALID_ACTION,
    PTRIE_STATUS_ENTRY_NOT_FOUND,
    PTRIE_STATUS_KEY_NOT_UNIQUE,
    PTRIE_STATUS_ENTRY_LIMIT_REACHED,
    PTRIE_STATUS_ALLOCATION_FAILED
} ptrie_status_t;


// ---------------------------------------------------------------------------
// Action callback function type
// ---------------------------------------------------------------------------

typedef void (*ptrie_action_f)(ptrie_key_t, ptrie_data_t);


// ---------------------------------------------------------------------------
// function:  ptrie_new_trie( status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new trie object.  Returns  NULL  if the trie object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

ptrie_t ptrie_new_trie(ptrie_status_t *status);


// ---------------------------------------------------------------------------
// function:  ptrie_store_entry( trie, key, value, status )
// ---------------------------------------------------------------------------
//
// Stores <value> for <key>  in <trie>.  The new entry is added  by reference,
// NO data is copied.  The function fails  if NULL is passed in  for <trie> or
// <key> or if a pointer to a zero length string is passed in for <key>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void ptrie_store_entry(ptrie_t trie,
                   ptrie_key_t key,
                  ptrie_data_t value,
                ptrie_status_t *status);


// ---------------------------------------------------------------------------
// function:  ptrie_replace_entry( trie, key, value, status )
// ---------------------------------------------------------------------------
//
// Searches the entry in <trie> whose key matches <key> and replaces its value
// with <value>.  The function fails if NULL is passed in for <trie> or <key>,
// or if a pointer to a  zero length string  is passed in for <key>,  or if no
// entry is found in <trie> with a key that matches <key>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void ptrie_replace_entry(ptrie_t trie,
                     ptrie_key_t key,
                    ptrie_data_t value,
                  ptrie_status_t *status);


// ---------------------------------------------------------------------------
// function:  ptrie_value_for_key( trie, index, status )
// ---------------------------------------------------------------------------
//
// Returns the value stored for <key> in <trie>.  If no value for <key> exists
// in <trie>,  or if NULL is passed in for <trie>,  then NULL is returned.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

ptrie_data_t ptrie_value_for_key(ptrie_t trie,
                             ptrie_key_t key,
                          ptrie_status_t *status);


// ---------------------------------------------------------------------------
// function:  ptrie_foreach_entry_do( trie, prefix, action, status )
// ---------------------------------------------------------------------------
//
// Traverses <trie>  visiting all entries whose keys have a common prefix with
// <prefix>  and invokes the  action callback function  passed in for <action>
// for each entry visited.  If  an  empty  string  is passed in  for <prefix>,
// then each entry in <trie> will be visited.  The function returns the number
// of entries visited.  The function fails and returns zero  if NULL is passed
// in for <trie> or <prefix> or <action>.
//
// Each time <action> is called,  the following parameters are passed to it:
//
// o  first parameter :  the key of the visited node
// o  second parameter:  the value of the visited node
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

ptrie_counter_t ptrie_foreach_entry_do(ptrie_t trie,
                                   ptrie_key_t prefix,
                                ptrie_action_f action,
                                ptrie_status_t *status);


// ---------------------------------------------------------------------------
// function:  ptrie_number_of_entries( trie )
// ---------------------------------------------------------------------------
//
// Returns the number of entries  stored in <trie>,  returns  zero  if NULL is
// passed in for <trie>.

inline ptrie_counter_t ptrie_number_of_entries(ptrie_t trie);


// ---------------------------------------------------------------------------
// function:  ptrie_number_of_entries_with_prefix( trie, prefix )
// ---------------------------------------------------------------------------
//
// Returns  the  number of entries  stored in <trie>  whose keys have a common
// prefix with <prefix>.  If an  empty string is passed in for <prefix>,  then
// the  total number  of entries  stored in <trie>  is returned.  The function
// fails and returns zero if NULL is passed in for <trie> or <key>.

ptrie_counter_t ptrie_number_of_entries_with_prefix(ptrie_t trie,
                                                ptrie_key_t prefix);


// ---------------------------------------------------------------------------
// function:  ptrie_remove_entry( trie )
// ---------------------------------------------------------------------------
//
// Removes the entry stored for <key> from <trie>.  The function fails if NULL
// is passed in for <trie> or if no entry for <key> is stored in <trie>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void ptrie_remove_entry(ptrie_t trie,
                    ptrie_key_t key,
                 ptrie_status_t *status);


// ---------------------------------------------------------------------------
// function:  ptrie_dispose_trie( trie )
// ---------------------------------------------------------------------------
//
// Disposes of trie object <trie>.  Returns NULL.

ptrie_t *ptrie_dispose_trie(ptrie_t trie);


#endif /* PTRIE_H */

// END OF FILE
