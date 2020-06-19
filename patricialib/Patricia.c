/* Patricia Trie Library
 *
 *  @file Patricia.h
 *  Patricia implementation
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

// ---------------------------------------------------------------------------
// Reference documents
// ---------------------------------------------------------------------------
//
// Patricia, Practical Algorithm To Retrieve Information Coded In Alphanumeric
// by Donald R.Morrison, published in JACM, Vol.15, Issue 4, 1968, by ACM
// Abstract:  http://portal.acm.org/citation.cfm?doid=321479.321481
//
// Use of a binary tree structure for processing files
// by G. Gwehenberger, published in Elektronische Rechenanlagen, No. 10, 1968
// Full text:  http://cr.yp.to/bib/1968/gwehenberger.html (in German only)


#include "Patricia.h"
#include "../common/alloc.h"


// ---------------------------------------------------------------------------
// Determine suitable unsigned integer type for bit index
// ---------------------------------------------------------------------------

#if (PTRIE_MAXIMUM_KEY_LENGTH * 8 <= ((1 << 8) - 1))
#define PTRIE_BIT_INDEX_BASE_TYPE uint8_t
#elif (PTRIE_MAXIMUM_KEY_LENGTH * 8 <= ((1 << 16) - 1))
#define PTRIE_BIT_INDEX_BASE_TYPE uint16_t
#elif (PTRIE_MAXIMUM_KEY_LENGTH * 8 <= ((1 << 32) - 1))
#define PTRIE_BIT_INDEX_BASE_TYPE uint32_t
#elif (PTRIE_MAXIMUM_KEY_LENGTH * 8 <= ((1 << 64UL) - 1))
#define PTRIE_BIT_INDEX_BASE_TYPE uint64_t
#else /* out of range for 64 bit unsigned */
#error PTRIE_MAXIMUM_KEY_LENGTH too large
#endif


// ---------------------------------------------------------------------------
// Bit index type
// ---------------------------------------------------------------------------

typedef PTRIE_BIT_INDEX_BASE_TYPE ptrie_index_t;


// ---------------------------------------------------------------------------
// Patricia trie node pointer type for self referencing declaration of node
// ---------------------------------------------------------------------------

struct _ptrie_node_s; /* FORWARD */

typedef struct _ptrie_node_s *ptrie_node_p;


// ---------------------------------------------------------------------------
// Patricia trie node type
// ---------------------------------------------------------------------------

struct _ptrie_node_s {
    ptrie_index_t index;
      ptrie_key_t key;
     ptrie_data_t value;
     ptrie_node_p left;
     ptrie_node_p right;
};

typedef struct _ptrie_node_s ptrie_node_s;


// ---------------------------------------------------------------------------
// Patricia trie type
// ---------------------------------------------------------------------------

typedef struct /* ptrie_s */ {
    ptrie_counter_t entry_count;
       ptrie_node_s *root_node;
} ptrie_s;


// ---------------------------------------------------------------------------
// Patricia trie node sentinel representing the bottom of a trie
// ---------------------------------------------------------------------------

static ptrie_node_s _bottom = { 0, NULL, NULL, &_bottom, &_bottom };

static ptrie_node_s *bottom = &_bottom;


// ===========================================================================
// P R I V A T E   F U N C T I O N   P R O T O T Y P E S
// ===========================================================================

static fmacro ptrie_node_p _search(ptrie_node_p start_node, ptrie_key_t key,
                                   bool *exact_match);

static ptrie_node_p _insert(ptrie_node_p node, ptrie_key_t key,
                            ptrie_data_t value, ptrie_status_t *status);

static void _remove(ptrie_node_p node, ptrie_key_t key,
                    ptrie_status_t *status);


// ===========================================================================
// P U B L I C   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// function:  ptrie_new_trie( status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new trie object.  Returns  NULL  if the trie object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

ptrie_t ptrie_new_trie(ptrie_status_t *status) {
    ptrie_s *new_trie;
        
    // allocate new trie
    new_trie = ALLOCATE(sizeof(ptrie_s));
    
    // bail out if allocation failed
    if (new_trie == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise new trie
    new_trie->entry_count = 0;
    new_trie->root_node = bottom;
    
    // pass new trie and status to caller
    ASSIGN_BY_REF(status, PTRIE_STATUS_SUCCESS);
    return (ptrie_t) new_trie;
} // end ptrie_new_trie


// ---------------------------------------------------------------------------
// function:  ptrie_store_entry( trie, key, value, status )
// ---------------------------------------------------------------------------
//
// Stores <value> for <key>  in <trie>.  The new entry is added  by reference,
// NO data is copied.  The function fails  if NULL is passed in  for <trie> or
// <value> or if zero is passed in for <key>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void ptrie_store_entry(ptrie_t trie,
                   ptrie_key_t key,
                  ptrie_data_t value,
                ptrie_status_t *status) {
    
    #define this_trie ((ptrie_s *)trie)
    ptrie_node_s *insertion_point;
    ptrie_node_s *new_node;
    ptrie_status_t r_status;
    bool exact_match;
    
    // bail out if trie is NULL
    if (trie == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_TRIE);
        return;
    } // end if
    
    // bail out if key is NULL or empty
    if ((key == NULL) || (key[0] == 0)) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_KEY);
        return NULL;
    } // end if
    
    insertion_point = _search(this_trie->root_node, key, &exact_match);
    
    // bail out if entry already exists
    if (exact_match == true) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_KEY_NOT_UNIQUE);
        return;
    } // end if
    
    this_trie->root_node = _insert(insertion_point, key, value, &r_status);
    
    if (r_status == PTRIE_STATUS_SUCCESS)
        this_trie->entry_count++;
    
    // pass status to caller
    ASSIGN_BY_REF(status, r_status);
    return;
    
    #undef this_trie
} // end ptrie_store_entry


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
                  ptrie_status_t *status) {
    
    #define this_trie ((ptrie_s *)trie)
    ptrie_node_s *this_node;
    bool exact_match;
    
    // bail out if trie is NULL
    if (trie == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_TRIE);
        return;
    } // end if
    
    // bail out if key is NULL or empty
    if ((key == NULL) || (key[0] == 0)) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_KEY);
        return NULL;
    } // end if
    
    this_node = _search(this_trie->root_node, key, &exact_match);
    
    if (exact_match == true) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_SUCCESS);
        this_node->value = value;
    }
    else /* not exact match */ {
        ASSIGN_BY_REF(status, PTRIE_STATUS_ENTRY_NOT_FOUND);
    } // end if
        
    return;
    
    #undef this_trie    
} // end ptrie_replace_entry


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
                          ptrie_status_t *status) {
    
    #define this_trie ((ptrie_s *)trie)
    ptrie_node_s *this_node;
    bool exact_match;
    
    // bail out if trie is NULL
    if (trie == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_TRIE);
        return NULL;
    } // end if
    
    // bail out if key is NULL or empty
    if ((key == NULL) || (key[0] == 0)) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_KEY);
        return NULL;
    } // end if
    
    this_node = _search(this_trie->root_node, key, &exact_match);
        
    if (exact_match == true) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_SUCCESS);
        return this_node->key;
    }
    else /* not exact match */ {
        ASSIGN_BY_REF(status, PTRIE_STATUS_ENTRY_NOT_FOUND);
        return NULL;
    } // end if
    
    #undef this_trie
} // end ptrie_value_for_key


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
                                ptrie_status_t *status) {
    
    #define this_trie ((ptrie_s *)trie)
    
    // bail out if trie is NULL
    if (trie == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_TRIE);
        return 0;
    } // end if
    
    // bail out if key is NULL
    if (key == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_KEY);
        return 0;
    } // end if

    // bail out if action is NULL
    if (action == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_ACTION);
        return 0;
    } // end if
    
    // TO DO
    
    #undef this_trie
} // end ptrie_foreach_entry_do


// ---------------------------------------------------------------------------
// function:  ptrie_number_of_entries( trie )
// ---------------------------------------------------------------------------
//
// Returns the number of entries  stored in <trie>,  returns  zero  if NULL is
// passed in for <trie>.

inline ptrie_counter_t ptrie_number_of_entries(ptrie_t trie) {
    #define this_trie ((ptrie_s *)trie)
    
    if (trie == NULL)
        return 0;
    
    return this_trie->entry_count;
    
    #undef this_trie
} // end ptrie_number_of_entries


// ---------------------------------------------------------------------------
// function:  ptrie_number_of_entries_with_prefix( trie, prefix )
// ---------------------------------------------------------------------------
//
// Returns  the  number of entries  stored in <trie>  whose keys have a common
// prefix with <prefix>.  If an  empty string is passed in for <prefix>,  then
// the  total number  of entries  stored in <trie>  is returned.  The function
// fails and returns zero if NULL is passed in for <trie> or <key>.

ptrie_counter_t ptrie_number_of_entries_with_prefix(ptrie_t trie,
                                                ptrie_key_t prefix) {
    
    #define this_trie ((ptrie_s *)trie)
    
    if (trie == NULL)
        return 0;

    // TO DO
    
    return 0;
    
    #undef this_trie
} // end ptrie_number_of_entries_with_prefix


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
                 ptrie_status_t *status) {
    
    #define this_trie ((ptrie_s *)trie)
    ptrie_status_t r_status;
    
    // bail out if trie is NULL
    if (trie == NULL) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_TRIE);
        return NULL;
    } // end if
    
    // bail out if key is NULL or empty
    if ((key == NULL) || (key[0] == 0)) {
        ASSIGN_BY_REF(status, PTRIE_STATUS_INVALID_KEY);
        return NULL;
    } // end if
    
    _remove(this_trie->root_node, key, &r_status);
    
    if (r_status == PTRIE_STATUS_SUCCESS)
        this_trie->entry_count--;
    
    // pass status to caller
    ASSIGN_BY_REF(status, r_status);
    return;
    
    #undef this_trie
} // end ptrie_remove_entry


// ---------------------------------------------------------------------------
// function:  ptrie_dispose_trie( trie )
// ---------------------------------------------------------------------------
//
// Disposes of trie object <trie>.  Returns NULL.

ptrie_t *ptrie_dispose_trie(ptrie_t trie) {
    #define this_trie ((ptrie_s *)trie)
    
    if (trie != NULL) {
        
        // deallocate all nodes
        _remove_all(this_trie->root_node);
        
        // deallocate the trie
        DEALLOCATE(this_trie);
    } // end if
    
    return NULL;
    
    #undef this_trie
} // end ptrie_dispose_trie


// ===========================================================================
// P R I V A T E   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// private macro:  BIT_AT_INDEX( key, index )
// ---------------------------------------------------------------------------
//
// Tests bit at <index>  in <key>  and evaluates to its value.  An index value
// of N denotes the position of bit (7 - N % 8) in byte (N / 8) of key.
//
// pre-conditions:
//
// o  index must not be less than 0
// o  index must not be larger than 8 * length of key - 1
// o  key must be a pointer to a C string or a byte array
//
// post-conditions:
//
// o  macro evaluates to 0 if bit at index in key is 0
// o  macro evaluates to 1 if but at index in key is 1
//
// error-conditions:
//
// o  undefined behaviour if pre-conditions are not met

#define BIT_AT_INDEX(_key, _index) \
    (((_key[_index / 8] & (7 - (index % 8))) == 0) ? 0 : 1)


// ---------------------------------------------------------------------------
// private function:  _search( start_node, search_key )
// ---------------------------------------------------------------------------
//
// Searches for the node whose key matches <search_key>.  The search starts at
// node <start_node>.  If an  exact match  is found,  then  the node whose key
// matches is returned  and true is passed back in <exact_match>.  If an exact
// match is not found,  then the node whose key is the closest match in <trie>
// is returned and false is passed back in <exact_match>.  NULL  must  NOT  be
// passed in for <start_node> or <search_key< or <exact_match>.

static fmacro ptrie_node_p _search(ptrie_node_p start_node,
                                    ptrie_key_t search_key,
                                           bool *exact_match) {
    ptrie_node_p this_node;
    ptrie_index_t index;
        
    this_node = start_node;
    
    // The bit index of a parent node is always larger
    // than the bit index of either of its child nodes.
    // to find an uplink, we move down the trie until the index of
    // a node is less than or equal to the index of its predecessor.
    
    repeat /* until uplink found */ {
        
        // remember bit index
        index = this_node->index;

        // test bit at index in search key
        if /* bit at index is 0 */ (BIT_AT_INDEX(search_key, index) == 0) {
            
            // move down the trie to the left
            this_node = this_node->left;
        }
        else /* bit at index is 1 */ {
            
            // move down the trie to the right
            this_node = this_node->right;
        } // end if
        
    } until (index >= this_node->index);
    
    // indicate exact match or best match
    *exact_match = (index == this_node->index);
            
    return this_node;
} // end _search


// ---------------------------------------------------------------------------
// private function:  _insert( node, key, value, status )
// ---------------------------------------------------------------------------
//
// Searches the trie starting at <node> for the insertion point for a new node
// with key <key>.  If no exact match exists,  then a  new node  is allocated,
// initialised with <key> and <value>,  then inserted  at the insertion point.
// If <node> is pointing  to the bottom sentinel node,  then <node> itself be-
// comes the insertion point.  NULL must NOT be passed in  for <node>,  <key>,
// or <status>.  The status of the operation is passed back in <status>.

static ptrie_node_p _insert(ptrie_node_p node,
                             ptrie_key_t key,
                            ptrie_data_t value,
                          ptrie_status_t *status) {
    
    ptrie_node_s *new_node;
        
    if (node == bottom) {
        
        // allocate new node
        new_node = ALLOCATE(sizeof(ptrie_node_s));
        
        // bail out if allocation failed
        if (new_node == NULL) {
            *status = PTRIE_STATUS_ALLOCATION_FAILED;
            return NULL;
        } // end if
        
        // initialise new node
        new_node->index = 0;
        new_node->key = key;
        new_node->value = value;
        new_node->left = bottom;
        new_node->right = new_node;
        
        // pass new node and status to caller
        *status = PTRIE_STATUS_SUCCESS;
        return new_node;
    } // end if
    
    // TO DO
    
    // return root node
    return node;
} // end _insert


// ---------------------------------------------------------------------------
// private function:  _remove( node )
// ---------------------------------------------------------------------------

static void _remove(ptrie_node_p node,
                     ptrie_key_t key,
                  ptrie_status_t *status) {
    
    // TO DO
    
    return;
} // end _remove


// ---------------------------------------------------------------------------
// private function:  _remove_all( node )
// ---------------------------------------------------------------------------
//
// Recursively deallocates all nodes in the trie whose root node is <node>.

static void _remove_all(ptrie_node_p node) {
    
    if (node == NULL)
        return;
    
    aat_remove_all(node->left);
    aat_remove_all(node->right);
    
    DEALLOCATE(node->key);
    DEALLOCATE(node);
    
    return;
} // end _remove_all


// END OF FILE
