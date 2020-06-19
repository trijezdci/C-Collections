/* BST Storage Library
 *
 *  @file Splay.c
 *  Splay implementation
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

// ---------------------------------------------------------------------------
// Reference documents
// ---------------------------------------------------------------------------
//
// Splay trees:  http://www.cs.cmu.edu/~sleator/papers/self-adjusting.pdf
// Sentinel search:  http://user.it.uu.se/~arnea/ps/searchproc.pdf


// ---------------------------------------------------------------------------
// Imports
// ---------------------------------------------------------------------------

#include "Splay.h"
#include "../common/alloc.h"


// ---------------------------------------------------------------------------
// Splay tree node pointer type for self referencing declaration of node
// ---------------------------------------------------------------------------

struct _splay_node_s; /* FORWARD */

typedef struct _splay_node_s *splay_node_p;


// ---------------------------------------------------------------------------
// Splay tree node type
// ---------------------------------------------------------------------------

struct _splay_node_s {
     splay_key_t key;
    splay_data_t value;
    splay_node_p left;
    splay_node_p right;
};

typedef struct _splay_node_s splay_node_s;


// ---------------------------------------------------------------------------
// Splay tree type
// ---------------------------------------------------------------------------

typedef struct /* splay_tree_s */ {
        cardinal entry_count;
    splay_node_p root_node;
} splay_tree_s;


// ---------------------------------------------------------------------------
// Splay tree node sentinel representing the bottom of a tree
// ---------------------------------------------------------------------------

static splay_node_s _bottom = { 0, NULL, &_bottom, &_bottom };

static splay_node_s *bottom = &_bottom;


// ===========================================================================
// P R I V A T E   F U N C T I O N   P R O T O T Y P E S
// ===========================================================================

static fmacro splay_node_p splay_top_down(splay_node_p node, splay_key_t key);

static fmacro splay_node_p splay_insert(splay_node_p node, splay_key_t key,
                                 splay_data_t value, splay_status_t *status);

static fmacro splay_node_p splay_remove(splay_node_p node, splay_key_t key,
                                 splay_status_t *status);

static void splay_remove_all(splay_node_p node);


// ===========================================================================
// P U B L I C   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// function:  splay_new_tree( status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new tree object.  Returns  NULL  if the tree object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

splay_tree_t splay_new_tree(splay_status_t *status) {
    
    splay_tree_s *new_tree;
    
    new_tree = ALLOCATE(sizeof(splay_tree_s));
    
    // bail out if allocation failed
    if (new_tree == NULL) {
        ASSIGN_BY_REF(status, SPLAY_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise new tree
    new_tree->entry_count = 0;
    new_tree->root_node = bottom;
    
    return (splay_tree_t) new_tree;
} // end splay_new_tree


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
                     splay_status_t *status) {
    
    #define this_tree ((splay_tree_s *)tree)
    splay_status_t r_status;
    
    // bail out if tree is NULL
    if (tree == NULL) {
        ASSIGN_BY_REF(status, SPLAY_STATUS_INVALID_TREE);
        return;
    } // end if
    
    // bail out if value is NULL
    if (value == NULL) {
        ASSIGN_BY_REF(status, SPLAY_STATUS_INVALID_DATA);
        return;
    } // end if
    
    // insert entry
    this_tree->root_node = 
        splay_insert(this_tree->root_node, key, value, &r_status);
    
    if (r_status == SPLAY_STATUS_SUCCESS)
        this_tree->entry_count++;
    
    ASSIGN_BY_REF(status, r_status);
    return;
    
    #undef this_tree
} // end splay_store_entry


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
                               splay_status_t *status) {
    
    #define this_tree ((splay_tree_s *)tree)
    splay_node_s *this_node;
    
    // bail out if tree is NULL
    if (tree == NULL) {
        ASSIGN_BY_REF(status, SPLAY_STATUS_INVALID_TREE);
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
        ASSIGN_BY_REF(status, SPLAY_STATUS_SUCCESS);
        return this_node->value;
    }
    else /* bottom reached means key not found */ {
        ASSIGN_BY_REF(status, SPLAY_STATUS_ENTRY_NOT_FOUND);
        return NULL;
    } // end if
    
    #undef this_tree
} // end splay_value_for_key


// ---------------------------------------------------------------------------
// function:  splay_number_of_entries( tree )
// ---------------------------------------------------------------------------
//
// Returns the number of entries  stored in <tree>,  returns  zero  if NULL is
// passed in for <tree>.

inline cardinal splay_number_of_entries(splay_tree_t tree) {
    
    #define this_tree ((splay_tree_s *)tree)
    
    // bail out if tree is NULL
    if (tree == NULL)
        return 0;
    
    return this_tree->entry_count;
    
    #undef this_tree
} // end splay_number_of_entries


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
                      splay_status_t *status) {
    
    #define this_tree ((splay_tree_s *)tree)
    splay_status_t r_status;
    
    // bail out if tree is NULL
    if (tree == NULL)
        return;
    
    // remove entry
    this_tree->root_node =
        splay_remove(this_tree->root_node, key, &r_status);
    
    if (r_status == SPLAY_STATUS_SUCCESS)
        this_tree->entry_count--;
    
    ASSIGN_BY_REF(status, r_status);
    return;
    
    #undef this_tree
} // end splay_remove_entry


// ---------------------------------------------------------------------------
// function:  splay_dispose_tree( tree )
// ---------------------------------------------------------------------------
//
// Disposes of tree object <tree>.  Returns NULL.

splay_tree_t *splay_dispose_tree(splay_tree_t tree) {
    
    #define this_tree ((splay_tree_s *)tree)
    
    // bail out if tree is NULL
    if (tree == NULL)
        return NULL;
    
    // deallocate all nodes
    splay_remove_all(this_tree->root_node);
    
    // deallocate tree
    DEALLOCATE(tree);
    return NULL;
    
    #undef this_tree
} // end splay_dispose_tree


// ===========================================================================
// P R I V A T E   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// private function:  splay_top_down( node )
// ---------------------------------------------------------------------------
//
// Rearranges the tree  whose root is <node> such  that the node  whose key is
// <key> moves to the top.  If no node with <key> is present in the tree, then
// the node that would be its closest neighbour is moved to the top instead.
//
// NULL must not be passed in for <node>.

static fmacro splay_node_p splay_top_down(splay_node_p node,
                                           splay_key_t key) {
    
    splay_node_s N, *temp_node, *left_subtree, *right_subtree;
    
    if (node == bottom)
        return node;
    
    N.left = bottom;
    N.right = bottom;
    left_subtree = &N;
    right_subtree = &N;
    
    loop {
        if (key < node->key) {
            
            if (node->left == bottom)
                break;
            
            // rotate right if key is less than current node's key
            if (key < node->left->key) {
                temp_node = node->left;
                node->left = temp_node->right;
                temp_node->right = node;
                node = temp_node;
                
                if (node->left == bottom)
                    break;
            } // end if
            
            // link right
            left_subtree->left = node;
            right_subtree = node;
            node = node->left;
        }
        else if (key > node->key) {
            
            if (node->right == bottom)
                break;
            
            // rotate left if key is greater than current node's key
            if (key > node->right->key) {
                temp_node = node->right;
                node->right = temp_node->left;
                temp_node->left = node;
                node = temp_node;
                
                if (node->right == bottom)
                    break;
            } // end if
            
            // link left
            left_subtree->right = node;
            left_subtree = node;
            node = node->right;
        }
        else /* key == node->key */ {
            break;
        } // end if
    } // end loop
    
    // assemble tree
    left_subtree->right = node->left;
    right_subtree->left = node->right;
    node->left = N.right;
    node->right = N.left;
        
    return node;
} // end splay_top_down


// ---------------------------------------------------------------------------
// private function:  splay_insert( node, key, status )
// ---------------------------------------------------------------------------
//
// Inserts a new entry for <key> with <value> into the tree whose root node is
// <node>.  Returns the  new root node  of the  resulting tree.  If allocation
// fails  or if a node with the same key already exists,  then  NO  entry will
// be inserted and the  current root node  is returned.
//
// The status of the operation  is passed back  in <status>.  NULL must not be
// passed in for <node> or <status>.

static fmacro splay_node_p splay_insert(splay_node_p node, splay_key_t key,
                                 splay_data_t value, splay_status_t *status) {
    splay_node_s *new_node;
    
    node = splay_top_down(node, key);
    
    // bail out if key already exists
    if (key == node->key) {
        *status = SPLAY_STATUS_KEY_NOT_UNIQUE;
        return node;
    }
    else {
        // allocate new node
        new_node = ALLOCATE(sizeof(splay_node_s));
        
        // bail out if allocation failed
        if (new_node == NULL) {
            *status = SPLAY_STATUS_ALLOCATION_FAILED;
            return node;
        } // end if
        
        // initialise new node
        new_node->key = key;
        new_node->value = value;
        
        if (key < node->key) {
            new_node->left = node->left;
            new_node->right = node;
            node->left = bottom;
        }
        else /* key > node->key */ {
            new_node->right = node->right;
            new_node->left = node;
            node->right = bottom;
        } // end if
    } // end if
    
    // return status and new root to caller
    *status = SPLAY_STATUS_SUCCESS;
    return new_node;
} // end splay_insert


// ---------------------------------------------------------------------------
// private function:  splay_remove( node, key, status )
// ---------------------------------------------------------------------------
//
// Searches the tree whose root node is <node>  for a node  whose key is <key>
// and if found,  removes that node  and  rebalances the resulting tree,  then
// returns the new root  of the resulting tree.  If no node with <key> exists,
// then the current root node is returned.
//
// The status of the operation  is passed back  in <status>.  NULL must not be
// passed in for <node> or <status>.

static fmacro splay_node_p splay_remove(splay_node_p node, splay_key_t key,
                                        splay_status_t *status) {
    splay_node_s *new_root;
    
    // bail out if bottom reached
    if (node == bottom) {
        *status = SPLAY_STATUS_ENTRY_NOT_FOUND;
        return node;
    } // end if
    
    node = splay_top_down(node, key);
    
    // bail out if key not found
    if (key != node->key) {
        *status = SPLAY_STATUS_ENTRY_NOT_FOUND;
        return node;
    } // end if
    
    // isolate node to be removed
    if (node->left == bottom) {
        new_root = node->right;
    }
    else {
        new_root = splay_top_down(node, key);
        new_root->right = node->right;
    } // end if
    
    // remove the node
    DEALLOCATE(node);
    
    // pass status and new root to caller
    *status = SPLAY_STATUS_SUCCESS;
    return new_root;
} // end splay_remove


// ---------------------------------------------------------------------------
// private function:  splay_remove_all( node )
// ---------------------------------------------------------------------------
//
// Recursively  removes  all nodes  from the tree  whose root node  is <node>.
// NULL must not be passed in for <node>.

static void splay_remove_all(splay_node_p node) {
    
    if (node == bottom)
        return;
    
    splay_remove_all(node->left);
    splay_remove_all(node->right);
    
    DEALLOCATE(node);
    
    return;
} // end splay_remove_all


// END OF FILE
