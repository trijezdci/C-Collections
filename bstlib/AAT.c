/* BST Storage Library
 *
 *  @file AAT.c
 *  AAT implementation
 *
 *  Universal AA Tree
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
// AA trees:  http://user.it.uu.se/~arnea/ps/simp.pdf
// Sentinel search:  http://user.it.uu.se/~arnea/ps/searchproc.pdf


// ---------------------------------------------------------------------------
// Imports
// ---------------------------------------------------------------------------

#include "AAT.h"
#include "../common/alloc.h"
#include "../common/bailout.h"


// ---------------------------------------------------------------------------
// AA tree node pointer type for self referencing declaration of node
// ---------------------------------------------------------------------------

struct _aat_node_s; /* FORWARD */

typedef struct _aat_node_s *aat_node_p;


// ---------------------------------------------------------------------------
// AA tree node type
// ---------------------------------------------------------------------------

struct _aat_node_s {
      cardinal level;
     aat_key_t key;
    aat_data_t value;
    aat_node_p left;
    aat_node_p right;
};

typedef struct _aat_node_s aat_node_s;


// ---------------------------------------------------------------------------
// AA tree type
// ---------------------------------------------------------------------------

typedef struct /* aat_tree_s */ {
      cardinal entry_count;
    aat_node_p root_node;
} aat_tree_s;


// ---------------------------------------------------------------------------
// AA tree node sentinel representing the bottom of a tree
// ---------------------------------------------------------------------------

static aat_node_s _bottom = { 0, 0, NULL, &_bottom, &_bottom };

static aat_node_s *bottom = &_bottom;


// ---------------------------------------------------------------------------
// Temporary node pointers for use during node removal
// ---------------------------------------------------------------------------

static aat_node_s *_aat_previous_node = NULL;

static aat_node_s *_aat_candidate_node = NULL;


// ===========================================================================
// P R I V A T E   F U N C T I O N   P R O T O T Y P E S
// ===========================================================================

static fmacro aat_node_p aat_skew(aat_node_p node);

static fmacro aat_node_p aat_split(aat_node_p node);

static aat_node_p aat_insert(aat_node_p node, aat_key_t key, aat_data_t value,
                             aat_status_t *status);

static aat_node_p aat_remove(aat_node_p node, aat_key_t key,
                             aat_status_t *status);

static void aat_remove_all(aat_node_p node);


// ===========================================================================
// P U B L I C   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// function:  aat_new_tree( status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new tree object.  Returns  NULL  if the tree object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

aat_tree_t aat_new_tree(aat_status_t *status) {
    
    aat_tree_s *new_tree;
    
    new_tree = ALLOCATE(sizeof(aat_tree_s));
    
    // bail out if allocation failed
    if (new_tree == NULL) {
        ASSIGN_BY_REF(status, AAT_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise new tree
    new_tree->entry_count = 0;
    new_tree->root_node = bottom;
    
    return (aat_tree_t) new_tree;
} // end aat_new_tree


// ---------------------------------------------------------------------------
// function:  aat_store_entry( tree, key, value, status )
// ---------------------------------------------------------------------------
//
// Stores <value> for <key>  in <tree>.  The new entry is added  by reference,
// NO data is copied.  The function fails  if NULL is passed in  for <tree> or
// <value> or if zero is passed in for <key>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void aat_store_entry(aat_tree_t tree,
                      aat_key_t key,
                     aat_data_t value,
                   aat_status_t *status) {
    
    #define this_tree ((aat_tree_s *)tree)
    aat_node_s *new_root;
    aat_status_t r_status;
    
    // bail out if tree is NULL
    if (tree == NULL) {
        ASSIGN_BY_REF(status, AAT_STATUS_INVALID_TREE);
        return;
    } // end if

    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, AAT_STATUS_INVALID_DATA);
        return;
    } // end if
            
    // insert entry
    new_root = aat_insert(this_tree->root_node, key, value, &r_status);
    
    if (r_status == AAT_STATUS_SUCCESS) {
        this_tree->root_node = new_root;
        this_tree->entry_count++;
    } // end if
    
    ASSIGN_BY_REF(status, r_status);
    return;
    
    #undef this_tree
} // end aat_store_entry


// ---------------------------------------------------------------------------
// function:  aat_value_for_key( tree, index, status )
// ---------------------------------------------------------------------------
//
// Returns the value stored for <key< in <tree>.  If no value for <key> exists
// in <tree>,  or if NULL is passed in for <tree>,  then NULL is returned.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

aat_data_t aat_value_for_key(aat_tree_t tree,
                              aat_key_t key,
                           aat_status_t *status) {
    
    #define this_tree ((aat_tree_s *)tree)
    aat_node_s *this_node;
    
    // bail out if tree is NULL
    if (tree == NULL) {
        ASSIGN_BY_REF(status, AAT_STATUS_INVALID_TREE);
        return NULL;
    } // end if
    
    // set sentinel's key to search key
    bottom->key = key;
    
    // start at the root
    this_node = this_tree->root_node;
    
    // search until key found
    while (key != this_node->key) {
        
        // move down left if key is less than key of current node
        if (key < this_node->key)
            this_node = this_node->left;
        
        // move down right if key is greater than key of current node
        else if (key > this_node->key)
            this_node = this_node->right;
    } // end while
    
    // reset sentinel's key
    bottom->key = 0;
    
    // check if bottom reached
    if (this_node != bottom) {
        ASSIGN_BY_REF(status, AAT_STATUS_SUCCESS);
        return this_node->value;
    }
    else /* bottom reached means key not found */ {
        ASSIGN_BY_REF(status, AAT_STATUS_ENTRY_NOT_FOUND);
        return NULL;
    } // end if
    
    #undef this_tree
} // end aat_value_for_key


// ---------------------------------------------------------------------------
// function:  aat_number_of_entries( tree )
// ---------------------------------------------------------------------------
//
// Returns the number of entries  stored in <tree>,  returns  zero  if NULL is
// passed in for <tree>.

inline cardinal aat_number_of_entries(aat_tree_t tree) {
    
    #define this_tree ((aat_tree_s *)tree)
    
    // bail out if tree is NULL
    if (tree == NULL)
        return 0;
        
    return this_tree->entry_count;
    
    #undef this_tree
} // end aat_number_of_entries


// ---------------------------------------------------------------------------
// function:  aat_remove_entry( tree )
// ---------------------------------------------------------------------------
//
// Removes the entry stored for <key> from <tree>.  The function fails if NULL
// is passed in for <tree> or if no entry for <key> is stored in <tree>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void aat_remove_entry(aat_tree_t tree,
                       aat_key_t key,
                    aat_status_t *status) {
    
    #define this_tree ((aat_tree_s *)tree)
    aat_node_s *new_root;
    aat_status_t r_status;
    
    // bail out if tree is NULL
    if (tree == NULL)
        return;
    
    // remove entry
    new_root = aat_remove(this_tree->root_node, key, &r_status);
    
    if (r_status == AAT_STATUS_SUCCESS) {
        this_tree->root_node = new_root;
        this_tree->entry_count--;
    } // end if
    
    ASSIGN_BY_REF(status, r_status);
    return;
    
    #undef this_tree
} // end aat_dispose_tree


// ---------------------------------------------------------------------------
// function:  aat_dispose_tree( tree )
// ---------------------------------------------------------------------------
//
// Disposes of tree object <tree>.  Returns NULL.

aat_tree_t *aat_dispose_tree(aat_tree_t tree) {
    
    #define this_tree ((aat_tree_s *)tree)
    
    // bail out if tree is NULL
    if (tree == NULL)
        return NULL;
        
    // deallocate all nodes
    aat_remove_all(this_tree->root_node);
        
    // deallocate tree
    DEALLOCATE(tree);
    return NULL;
    
    #undef this_tree
} // end aat_dispose_tree


// ===========================================================================
// P R I V A T E   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// private function:  aat_skew( node )
// ---------------------------------------------------------------------------
//
// Rotates <node> to the right if its left child has the same level as <node>.
// NULL must not be passed in for <node>.

static fmacro aat_node_p aat_skew(aat_node_p node) {
    aat_node_s *temp_node;
    
    // rotate right if left child has same level
    if (node->level == node->left->level) {
        temp_node = node;
        node = node->left;
        temp_node->left = node->right;
        node->right = temp_node;
    } // end if
    
    return node;
} // end aat_skew


// ---------------------------------------------------------------------------
// private function:  aat_split( node )
// ---------------------------------------------------------------------------
//
// Rotates <node> left and promotes the level of its right child to become its
// new parent if <node> has two consecutive right children with the same level
// level as <node>.  NULL must not be passed in for <node>.

static fmacro aat_node_p aat_split(aat_node_p node) {
    aat_node_s *temp_node;
    
    // rotate left if there are two right children on same level
    if (node->level == node->right->right->level) {
        temp_node = node;
        node = node->right;
        temp_node->right = node->left;
        node->right = temp_node;
        node->level++;
    } // end if
    
    return node;
} // end aat_split


// ---------------------------------------------------------------------------
// private function:  aat_insert( node, key, status )
// ---------------------------------------------------------------------------
//
// Recursively inserts  a new entry for <key> with <value> into the tree whose
// root node is <node>.  Returns the new root node  of the resulting tree.  If
// allocation fails  or  if a node with the same key already exists,  then  NO
// entry will be inserted and NULL is returned.
//
// The status of the operation  is passed back  in <status>.  NULL must not be
// passed in for <node> or <status>.

static aat_node_p aat_insert(aat_node_p node, aat_key_t key, aat_data_t value,
                             aat_status_t *status) {
    aat_node_s *new_node;
    
    if (node == bottom) {
        // allocate a new node
        new_node = ALLOCATE(sizeof(aat_node_s));
        
        // bail out if allocation failed
        if (new_node == NULL)
            BAILOUT(allocation_failed);
        
        // initialise new node
        new_node->level = 1;
        new_node->key = key;
        new_node->value = value;
        new_node->left = bottom;
        new_node->right = bottom;
        
        // link new node to the tree
        node = new_node;
    }
    else if (node->key > key) {
        node = aat_insert(node->left, key, value, status);
        
        // bail out if allocation failed
        if (*status == AAT_STATUS_ALLOCATION_FAILED)
            BAILOUT(allocation_failed);
    }
    else if (node->key < key) {
        node = aat_insert(node->right, key, value, status);
        
        // bail out if allocation failed
        if (*status == AAT_STATUS_ALLOCATION_FAILED)
            BAILOUT(allocation_failed);
    }
    else /* key already exists */ {
        BAILOUT(key_not_unique);
    } // end if
    
    node = aat_skew(node);
    node = aat_split(node);
    
    /* NORMAL TERMINATION */
    
    *status = AAT_STATUS_SUCCESS;
    return node;
    
    /* ERROR HANDLING */
    
    ON_ERROR(allocation_failed) :
    
    *status = AAT_STATUS_ALLOCATION_FAILED;
    return NULL;
    
    ON_ERROR(key_not_unique) :
    
    *status = AAT_STATUS_KEY_NOT_UNIQUE;
    return NULL;
} // end aat_insert


// ---------------------------------------------------------------------------
// private function:  aat_remove( node, key, status )
// ---------------------------------------------------------------------------
//
// Recursively searches the tree  whose root node is <node>  for a node  whose
// key is <key> and if found,  removes that node  and rebalances the resulting
// tree,  then  returns  the new root  of the resulting tree.  If no node with
// <key> exists,  then NULL is returned.
//
// The status of the operation  is passed back  in <status>.  NULL must not be
// passed in for <node> or <status>.

static aat_node_p aat_remove(aat_node_p node, aat_key_t key,
                             aat_status_t *status) {
    
    // bail out if bottom reached
    if (node == bottom) {
        *status = AAT_STATUS_ENTRY_NOT_FOUND;
        return NULL;
    } // end if
    
    // move down recursively until key is found or bottom reached
    _aat_previous_node = node;
    
    // move left if search key less than current node's key
    if (key < node->key) {
        node = aat_remove(node->left, key, status);
    }
    // mode right if search key not less than current node's key
    else {
        _aat_candidate_node = node;
        node = aat_remove(node->right, key, status);
    } // end if
    
    // remove entry
    if ((node == _aat_previous_node) &&
        (_aat_candidate_node != bottom) && (_aat_candidate_node->key == key)) {
        
        _aat_candidate_node->key = node->key;
        _aat_candidate_node = bottom;
        node = node->right;
        
        DEALLOCATE(_aat_previous_node);
        *status = AAT_STATUS_SUCCESS;
    }
    // rebalance on the way back up
    else if ((node->level-1 > node->left->level) ||
             (node->level-1 < node->right->level)) {
        
        node->level = node->level--;
        if (node->level < node->right->level)
            node->right->level = node->level;
        
        node = aat_skew(node);
        node = aat_skew(node->right);
        node = aat_skew(node->right->right);
        node = aat_split(node);
        node = aat_split(node->right);
    } // end if
    
    return node;
} // end aat_remove


// ---------------------------------------------------------------------------
// private function:  aat_remove_all( node )
// ---------------------------------------------------------------------------
//
// Recursively  removes  all nodes  from the tree  whose root node  is <node>.
// NULL must not be passed in for <node>.

static void aat_remove_all(aat_node_p node) {
    
    if (node == bottom)
        return;
    
    aat_remove_all(node->left);
    aat_remove_all(node->right);
    
    DEALLOCATE(node);
    
    return;
} // end aat_remove_all


// END OF FILE
