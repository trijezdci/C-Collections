/* Key Value Storage Library
 *
 *  @file KVS.c
 *  KVS implementation
 *
 *  Universal Associative Array
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
 *  Version history:
 *
 *   1.00   2009-09-25   BK   new file
 */


#include "../common/alloc.h"
#include "KVS.h"


// ---------------------------------------------------------------------------
// Range checks
// ---------------------------------------------------------------------------

#if (KVS_DEFAULT_TABLE_SIZE <= 0)
#error "Fatal error: KVS_DEFAULT_TABLE_SIZE must be greater than zero."
#endif

#if (KVS_MAX_STRING_SIZE > 0x0ffffffff)
#error "Fatal error: KVS_MAX_STRING_SIZE must not exceed 32 bit."
#endif


// ---------------------------------------------------------------------------
// KVS table entry pointer type
// ---------------------------------------------------------------------------

struct _kvs_entry_s; /* FORWARD */

typedef struct _kvs_entry_s *kvs_entry;


// ---------------------------------------------------------------------------
// KVS table entry type
// ---------------------------------------------------------------------------

struct _kvs_entry_s {
    kvs_key_t key; // unique 32 bit key
     opaque_t value; // pointer to stored data
     cardinal size; // allocation size in bytes
    kvs_entry next;  // next entry in same bucket
       word_t ref_count; // entry's reference count
      octet_t null_terminated; // must be true or false
      octet_t marked_for_removal; // must be true or false
};

typedef struct _kvs_entry_s kvs_entry_s;


// ---------------------------------------------------------------------------
// KVS table type
// ---------------------------------------------------------------------------

typedef struct /* kvs_table_s */ {
     kvs_entry last_retrieved_entry;
      cardinal entry_count;
      cardinal bucket_count;
     kvs_entry bucket[0];
} kvs_table_s;


// ===========================================================================
// P R I V A T E   F U N C T I O N   P R O T O T Y P E S   A N D   M A C R O S
// ===========================================================================

#define _kvs_set_status(_status_p,_code) \
    { if (_status_p != NULL) *_status_p = _code; }

static kvs_entry _kvs_find_entry
    (kvs_table_t table, kvs_key_t key, kvs_status_t *status);

static fmacro cardinal _kvs_calc_null_terminated_data_size
    (kvs_data_t data);

static fmacro kvs_entry _kvs_new_entry_with_copy
    (kvs_key_t key, kvs_data_t value, cardinal size,
     bool null_terminated, kvs_status_t *status);

static fmacro kvs_entry _kvs_new_entry_with_ref
    (kvs_key_t key, kvs_data_t value, cardinal size,
     bool null_terminated, kvs_status_t *status);

static fmacro kvs_data_t _kvs_retrieve_copy
    (kvs_entry_s *entry, kvs_status_t *status);

static fmacro void _kvs_dispose_entry
    (kvs_entry entry, kvs_status_t *status);


// ===========================================================================
// P U B L I C   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// function:  kvs_new_table( size, status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new KVS table object with <size> number of buckets.
// If  zero is passed in <size>,  then  the new table will be created with the
// default table size  as  defined  by  KVS_DEFAULT_TABLE_SIZE.  Returns  NULL
// if the KVS table object could not be created.

// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

kvs_table_t kvs_new_table(cardinal size, kvs_status_t *status) {

    cardinal index, bucket_count;
    kvs_table_s *new_table;
    
    // determine table size
    if (size == 0) bucket_count = KVS_DEFAULT_TABLE_SIZE;
    else bucket_count = size;
    
    // allocate table base
    new_table =
        ALLOCATE(sizeof(kvs_table_s) + sizeof(kvs_entry) * (bucket_count - 1));
    
    // exit if allocation failed
    if (new_table == NULL) {
        _kvs_set_status(status, KVS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise table meta data
    new_table->last_retrieved_entry = NULL;
    new_table->entry_count = 0;
    new_table->bucket_count = bucket_count;
    
    // initialise buckets with NULL pointers
    for (index = 0; index < bucket_count; index++) {
        new_table->bucket[index] = NULL;
    } // end for
    
    // set status
    _kvs_set_status(status, KVS_STATUS_SUCCESS);
    
    // return a reference to the new table
    return (kvs_table_t) new_table;
} // end kvs_new_table


// ---------------------------------------------------------------------------
// function:  kvs_store_value( table, key, val, size, null_terminated, stat )
// ---------------------------------------------------------------------------
//
// Adds a new entry  for key <key>  to table <table>.  The new entry is stored
// by value.  Data is copied from the address passed in as <value>.  If <size>
// is not zero, then <size> number of bytes are copied.  If <size> is zero and
// <null_terminated> is true, then data will be copied up to and including the
// first zero-value byte encountered.  If <size> is zero and <null_terminated>
// is false,  then the operation will fail and no entry will be added.  If the
// operation succeeds,  then the initial reference count of the new entry will
// be set to one.
//
// Keys must be unique.  Existing entries are not replaced.  Duplicate entries
// are not added.  The  status  of  the operation  is passed back in <status>,
// unless NULL was passed in for <status>.

void kvs_store_value(kvs_table_t table,
                       kvs_key_t key,
                      kvs_data_t value,
                        cardinal size,
                       bool null_terminated,
                    kvs_status_t *status) {
    cardinal index;
    kvs_table_s *this_table = (kvs_table_s *) table;
    kvs_entry_s *new_entry = NULL;
    kvs_entry_s *this_entry = NULL;
    kvs_status_t _status;
   
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return;
    } // end if
    
    // key must not be zero
    if (key == 0) {
        _kvs_set_status(status, KVS_STATUS_INVALID_KEY);
        return;
    } // end if
    
    // value must not be NULL
    if (value == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_DATA);
        return;
    } // end if
    
    // size must not be NULL if data is not null-terminated
    if ((size == 0) && (null_terminated == false)) {
        _kvs_set_status(status, KVS_STATUS_INVALID_SIZE);
        return;
    } // end if
    
    if /* size unknown */ (size == 0) {
        if /* data is null-terminated */ (null_terminated) {
            // calculate size
            size = _kvs_calc_null_terminated_data_size(value);
            if (size == 0) {
                _kvs_set_status(status, KVS_STATUS_INVALID_SIZE);
                return;
            } // end if
        }
        else /* data is not null-terminated */ {
            _kvs_set_status(status, KVS_STATUS_INVALID_SIZE);
            return;
        } // end if
    } // end if
    
    // calculate the bucket index for key
    index = key % this_table->bucket_count;

    if /* bucket is empty */ (this_table->bucket[index] == NULL) {
        
        // create a new entry
        new_entry =
        _kvs_new_entry_with_copy(key, value, size, null_terminated, &_status);
        
        // exit if allocation failed
        if (new_entry == NULL) {
            _kvs_set_status(status, _status);
            return;
        } // end if
        
        // link the empty bucket to the new entry
        this_table->bucket[index] = new_entry;
        
        // update the entry counter
        this_table->entry_count++;
        
        // set status
        _kvs_set_status(status, KVS_STATUS_SUCCESS);
    }
    else /* bucket is not empty */ {
        
        // first entry in this bucket is starting point
        this_entry = (kvs_entry) this_table->bucket[index];
        
        // check every entry in this bucket for a key match
        while ((this_entry->key != key) && (this_entry->next != NULL))
            this_entry = this_entry->next;
        
        // the passed in key is unique if there was no key match
        
        if /* new key is unique */ (this_entry->key != key) {
            
            // create a new entry
            new_entry =
            _kvs_new_entry_with_copy(key, value, size, null_terminated, &_status);
            
            // exit if allocation failed
            if (new_entry == NULL) {
                _kvs_set_status(status, _status);
                return;
            } // end if
            
            // link the final entry in the chain to the new entry
            this_entry->next = new_entry;
            
            // update the entry counter
            this_table->entry_count++;
            
            // set status
            _kvs_set_status(status, KVS_STATUS_SUCCESS);
        }
        else /* key is not unique */ {
            
            // do not add a new entry
            
            // set status
            _kvs_set_status(status, KVS_STATUS_KEY_NOT_UNIQUE);
        } // end if
    } // end if
    
    return;
} // end kvs_store_value


// ---------------------------------------------------------------------------
// function:  kvs_store_reference( tbl, key, val, siz, null_terminated, stat )
// ---------------------------------------------------------------------------
//
// Adds a new entry  for key <key>  to table <table>.  The new entry is stored
// by reference.  No data is copied.  If <size> is zero  and <null_terminated>
// is true,  then  the size of the referenced data  is calculated  by counting
// up to  and including  the  first  zero-value byte.  The size of the data is
// then stored for faster retrieval by-copy of the entry.  Entries  stored  by
// reference  and for which  the size  is unknown cannot be retrieved by copy.
// The initial reference count of the new entry will be set to one.
//
// Keys must be unique.  Existing entries are not replaced.  Duplicate entries
// are not added.  The  status  of  the operation  is passed back in <status>,
// unless NULL was passed in for <status>.

void kvs_store_reference(kvs_table_t table,
                           kvs_key_t key,
                          kvs_data_t value,
                            cardinal size,
                           bool null_terminated,
                        kvs_status_t *status) {
    cardinal index;
    kvs_table_s *this_table = (kvs_table_s *) table;
    kvs_entry_s *new_entry = NULL;
    kvs_entry_s *this_entry = NULL;
    kvs_status_t _status;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return;
    } // end if
    
    // key must not be zero
    if (key == 0) {
        _kvs_set_status(status, KVS_STATUS_INVALID_KEY);
        return;
    } // end if
    
    // value must not be NULL
    if (value == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_DATA);
        return;
    } // end if
    
    // size must not be NULL if data is not null-terminated
    if ((size == 0) && (null_terminated == false)) {
        _kvs_set_status(status, KVS_STATUS_INVALID_SIZE);
        return;
    } // end if
    
    if /* size unknown */ ((size == 0) && (null_terminated)) {
        // calculate size
        size = _kvs_calc_null_terminated_data_size(value);
        if (size == 0) {
            _kvs_set_status(status, KVS_STATUS_INVALID_SIZE);
            return;
        } // end if
    } // end if
    
    // calculate the bucket index for key
    index = key % this_table->bucket_count;
    
    if /* bucket is empty */ (this_table->bucket[index] == NULL) {
        
        // create a new entry
        new_entry =
        _kvs_new_entry_with_ref(key, value, size, null_terminated, &_status);
        
        // exit if allocation failed
        if (new_entry == NULL) {
            _kvs_set_status(status, _status);
            return;
        } // end if
        
        // link the empty bucket to the new entry
        this_table->bucket[index] = new_entry;
        
        // update the entry counter
        this_table->entry_count++;
        
        // set status
        _kvs_set_status(status, KVS_STATUS_SUCCESS);
    }
    else /* bucket is not empty */ {
        
        // first entry in this bucket is starting point
        this_entry = (kvs_entry) this_table->bucket[index];
        
        // check every entry in this bucket for a key match
        while ((this_entry->key != key) && (this_entry->next != NULL))
            this_entry = this_entry->next;
        
        // the passed in key is unique if there was no key match
        
        if /* new key is unique */ (this_entry->key != key) {
            
            // create a new entry
            new_entry =
            _kvs_new_entry_with_ref(key, value, size, null_terminated, &_status);
            
            // exit if allocation failed
            if (new_entry == NULL) {
                _kvs_set_status(status, _status);
                return;
            } // end if
            
            // link the final entry in the chain to the new entry
            this_entry->next = new_entry;
            
            // update the entry counter
            this_table->entry_count++;
            
            // set status
            _kvs_set_status(status, KVS_STATUS_SUCCESS);
        }
        else /* key is not unique */ {
            
            // do not add a new entry
            
            // set status
            _kvs_set_status(status, KVS_STATUS_KEY_NOT_UNIQUE);
        } // end if
    } // end if
    
    return;
} // end kvs_store_reference


// ---------------------------------------------------------------------------
// function:  kvs_entry_exists( table, key, status )
// ---------------------------------------------------------------------------
//
// Returns  true  if a  valid entry  for <key> exists  in  KVS  table <table>,
// returns false otherwise.  If an entry is found,  valid or invalid,  then it
// will be cached internally  and a subsequent search request for the same key
// will check the cached entry first, which is slighly faster than a lookup of
// a  non-cached entry.  The  reference count  of the entry is  not  modified.
// The  status  of the operation  is passed back in <status>,  unless NULL was
// passed in for <status>.

bool kvs_entry_exists(kvs_table_t table,
                        kvs_key_t key, 
                     kvs_status_t *status) {

    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return false;
    } // end if

    // try to find entry for key
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry not found */ (this_entry == NULL) {
        return false;
    }
    else if (this_entry->marked_for_removal) {
        _kvs_set_status(status, KVS_STATUS_ENTRY_PENDING_REMOVAL);
        return false;
    }
    else /* entry is valid */ {
        return true;
    } // end if
    
} // end kvs_entry_exists


// ---------------------------------------------------------------------------
// function:  kvs_get_entry(table, copy, key, size, null_terminated, status)
// ---------------------------------------------------------------------------
//
// Retrieves the table entry stored in <table> for <key>  either by copy or by
// reference.  If  true  is passed in for <copy>,  then  the  function returns
// the entry's data by-copy,  otherwise it returns the data by-reference.
//
// For by-copy retrieval,  if the entry exists,  a newly allocated copy of its
// value is created,  and a pointer to it is returned as function result.  The
// size of the entry's data (in bytes)  is passed back in <size>.  However, if
// the size of the entry's data is unknown,  then no copy is made and  NULL is
// returned.  The entry's reference count is never incremented when retrieving
// by copy.
//
// For by-reference retrieval,  if the entry exists,  a pointer to the entry's
// data  is returned as function result  and  the  entry's reference count  is
// incremented.  The size of the  entry's data  (in bytes)  is  passed back in
// <size>.  However,  if the size of the entry's data is  unknown,  then  zero
// is passed back in <size>.
//
// If the entry's data is null-terminated,  then  true  will be passed back in
// <null_terminated>,  otherwise  false  will be passed back.
//
// If the entry has been successfully retrieved,  then it is cached within the
// table,  regardless of whether it was returned by copy or by reference.
//
// If the entry does not exist,  or,  if it has been marked for removal,  then
// NULL  is  returned,  no  data  is copied,  no table meta data  is modified,
// no entry meta data is modified.
//
// The  status  of the operation  is passed back in <status>,  unless NULL was
// passed in for <status>.

kvs_data_t kvs_get_entry(kvs_table_t table,
                                bool copy,
                           kvs_key_t key,
                            cardinal *size,
                                bool *null_terminated,
                        kvs_status_t *status) {
    /* octet_t *retrieved_value = NULL; */
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return NULL;
    } // end if
    
    // try to find entry for key
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry found */ (this_entry != NULL) {
        
        // check if entry is pending removal
        if (this_entry->marked_for_removal) {
        
            // set status and return NULL
            _kvs_set_status(status, KVS_STATUS_ENTRY_PENDING_REMOVAL);
            return NULL;
        }
        else if /* by copy */ (copy == true) {
            
            if /* entry size unknown */ (this_entry->size == 0) {
                
                // set status and return NULL
                _kvs_set_status(status, KVS_STATUS_SIZE_OF_ENTRY_UNKNOWN);
                return NULL;
            }
            else /* entry not pending removal and size known */ {
                
                // pass back size and null_terminated
                if (size != NULL)
                    *size = this_entry->size;
                if (null_terminated != NULL)
                    *null_terminated = this_entry->null_terminated;
                
                // make a copy and return it
                return _kvs_retrieve_copy(this_entry, status);
            } // end if
        }
        else /* by reference */ {
            
            // pass back size and null_terminated
            if (size != NULL)
                *size = this_entry->size;
            if (null_terminated != NULL)
                *null_terminated = this_entry->null_terminated;

            // increment the reference count for entry
            this_entry->ref_count++;
            
            // set status and return reference
            _kvs_set_status(status, KVS_STATUS_SUCCESS);
            return (kvs_data_t) this_entry->value;
        } // end if
    }
    else /* entry not found */ {
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND);
        return NULL;
    } // end if

} // end kvs_get_entry


// ---------------------------------------------------------------------------
// function:  kvs_value_for_key( table, key, status )
// ---------------------------------------------------------------------------
//
// Retrieves the entry stored in table <table> for <key> and returns a pointer
// to a  newly  allocated  copy  of the  entry's value.  The entry's reference
// count is NOT incremented.  Entries that have been stored  by reference  and
// are not null-terminated can  only  be retrieved by value if their data size
// was  explicitly  passed in  when  they  were  stored.  If  no entry  exists
// for <key>  or if the entry is pending removal  or  if the size of the entry
// is unknown,  then  NULL  is  returned.  The  status  of  the  operation  is
// passed  back in <status>,  unless  NULL  was passed in for <status>.

kvs_data_t kvs_value_for_key(kvs_table_t table,
                               kvs_key_t key,
                            kvs_status_t *status) {
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return NULL;
    } // end if
    
    // try to find entry for key
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry found */ (this_entry != NULL) {
        
        if /* pending removal */ (this_entry->marked_for_removal) {
            
            // set status and return NULL
            _kvs_set_status(status, KVS_STATUS_ENTRY_PENDING_REMOVAL);
            return NULL;
        }
        else if /* entry's size is unknown */ (this_entry->size == 0) {
            
            // set status and return NULL
            _kvs_set_status(status, KVS_STATUS_SIZE_OF_ENTRY_UNKNOWN);
            return NULL;
        }
        else /* entry not pending removal and size known */ {
            
            // make a copy and return it
            return _kvs_retrieve_copy(this_entry, status);
        } // end if
    }
    else /* entry not found */ {
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND);
        return NULL;
    } // end if
    
} // end kvs_value_for_key


// ---------------------------------------------------------------------------
// function:  kvs_reference_for_key( table, key, status )
// ---------------------------------------------------------------------------
//
// Retrieves the entry stored in table <table> for <key> and returns a pointer
// to the  entry's value  in the table  and  increments the  entry's reference
// count.  If  no entry exists  for <key>  or if the entry is pending removal,
// then  NULL  is returned.  The  status  of  the  operation  is  passed  back
// in <status>,  unless  NULL  was passed in for <status>.

kvs_data_t kvs_reference_for_key(kvs_table_t table,
                                   kvs_key_t key,
                                kvs_status_t *status) {
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return NULL;
    } // end if
    
    // try to find entry for key
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry found */ (this_entry != NULL) {
        
        if /* pending removal */ (this_entry->marked_for_removal) {
            
            // set status and return NULL
            _kvs_set_status(status, KVS_STATUS_ENTRY_PENDING_REMOVAL);
            return NULL;
        }
        else /* not pending removal */ {
            
            // increment the reference count
            this_entry->ref_count++;
                        
            // set status and return reference
            _kvs_set_status(status, KVS_STATUS_SUCCESS);
            return (kvs_data_t) this_entry->value;
        } // end if
    }
    else /* entry not found */ {
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND);
        return NULL;
    } // end if
    
} // end kvs_reference_for_key


// ---------------------------------------------------------------------------
// function:  kvs_size_for_key( table, key, status )
// ---------------------------------------------------------------------------
//
// Returns  the size of the data of the entry stored in <table> for <key>.  If
// no entry exists for <key>,  then  zero  is  returned.  The  status  of  the
// operation  is  passed  back  in  <status>,  unless   NULL   was  passed  in
// for <status>.

cardinal kvs_size_for_key(kvs_table_t table,
                          kvs_key_t key,
                          kvs_status_t *status) {
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return 0;
    } // end if
    
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry found */ (this_entry != NULL)
        return this_entry->size;
    else /* entry not found */
        return 0;
} // end kvs_size_for_key


// ---------------------------------------------------------------------------
// function:  kvs_data_for_key_is_null_terminated( table, key, status )
// ---------------------------------------------------------------------------
//
// Returns the  null-terminated flag of the entry stored in <table> for <key>.
// If no entry exists for <key>,  then  false  is returned.  The status of the
// operation  is  passed  back  in  <status>,  unless   NULL   was  passed  in
// for <status>.

bool kvs_data_for_key_is_null_terminated(kvs_table_t table,
                                           kvs_key_t key,
                                        kvs_status_t *status) {
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return false;
    } // end if
    
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry found */ (this_entry != NULL)
        return this_entry->null_terminated;
    else /* entry not found */
        return false;
} // end kvs_data_for_key_is_null_terminated


// ---------------------------------------------------------------------------
// function:  kvs_reference_count_for_key( table, key, status )
// ---------------------------------------------------------------------------
//
// Returns the  reference count  of the entry stored in <table> for <key>.  If
// no entry exists for <key>,  then  zero  is returned.  Valid entries  always
// have a  reference count  greater than zero.  The status of the operation is
// passed back in <status>,  unless  NULL  was passed in for <status>.

cardinal kvs_reference_count_for_key(kvs_table_t table,
                                       kvs_key_t key,
                                    kvs_status_t *status) {
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return 0;
    } // end if
    
    this_entry = _kvs_find_entry(table, key, status);
    
    if /* entry found */ (this_entry != NULL)
        return this_entry->ref_count;
    else /* entry not found */
        return 0;
} // end kvs_reference_count_for_key


// ---------------------------------------------------------------------------
// function:  kvs_release_entry( table, key, status )
// ---------------------------------------------------------------------------
//
// Decrements the reference count of entry stored in <table> for <key> by one.
// If the entry has previously been marked for removal and its reference count
// reaches one  as a result of this release,  then  the entry will be removed.
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void kvs_release_entry(kvs_table_t table,
                         kvs_key_t key,
                      kvs_status_t *status) {
    kvs_entry this_entry;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return;
    } // end if
    
    this_entry = _kvs_find_entry(table, key, status);
        
    if (this_entry != NULL) {
        if (this_entry->ref_count > 1) {
            this_entry->ref_count--;
            
            if ((this_entry->ref_count == 1) &&
                (this_entry->marked_for_removal)) {
                
                kvs_remove_entry(table, key, status);
                
            } // end if
        } // end if
    } // end if
    
    return;
} // end kvs_release_entry


// ---------------------------------------------------------------------------
// function:  kvs_remove_entry(table, key, status)
// ---------------------------------------------------------------------------
//
// Marks the entry stored in <table> for <key> as removed.  An entry which has
// been marked as removed can no longer be retrieved  and will be removed when
// its  reference count  reaches  zero.  The status of the operation is passed
// back in <status>,  unless  NULL  was passed in for <status>.

void kvs_remove_entry(kvs_table_t table,
                        kvs_key_t key,
                     kvs_status_t *status) {
    cardinal index;
    kvs_entry prev_entry, this_entry;
    kvs_table_s *this_table = (kvs_table_s *) table;
    
    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return;
    } // end if
    
    // key must not be zero
    if (key == 0) {
        _kvs_set_status(status, KVS_STATUS_INVALID_KEY);
        return;
    } // end if
    
    // calculate the bucket index for key
    index = key % this_table->bucket_count;

    if /* bucket is empty */ (this_table->bucket[index] == NULL) {
    
        // entry not found
        
        // set status
        _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND); 
    }
    else /* bucket not empty */ {
        
        // starting point
        this_entry = this_table->bucket[index];
        prev_entry = this_entry;
        
        // move to next entry until key matches or last entry is reached
        while ((this_entry->key != key) && (this_entry->next != NULL)) {
            prev_entry = this_entry;
            this_entry = this_entry->next;
        } // end while

        if /* key matched */ (this_entry->key == key) {
            
            // only remove if reference count is 1 (or less, just in case)
            if (this_entry->ref_count <= 1) {
                
                // if chached, remove the entry from the cache
                if (this_table->last_retrieved_entry == this_entry)
                    this_table->last_retrieved_entry = NULL;
                
                // remove the entry from the bucket
                if /* first entry */ (this_table->bucket[index] == this_entry) {
                    // link bucket root to successor
                    this_table->bucket[index] = this_entry->next;
                }
                else /* not first entry */ {
                    // link predecessor to successor
                    prev_entry->next = this_entry->next;
                } // end if
            
                // deallocate the entry
                _kvs_dispose_entry(this_entry, status);
                        
                // update the entry counter
                this_table->entry_count--;
            }
            else /* reference count > 1 */ {
            
                // don't remove the entry yet, mark it for removal
                this_entry->marked_for_removal = true;
                
                // set status
                _kvs_set_status(status, KVS_STATUS_SUCCESS);
                
            } // end if
        }
        else /* key did not match any key in the bucket */ {
            
            // entry not found
            
            // set status
            _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND); 

        } // end if
    } // end if

    return;
} // end kvs_remove_entry


// ---------------------------------------------------------------------------
// function:  kvs_number_of_buckets( table )
// ---------------------------------------------------------------------------
//
// Returns  the number of buckets  of KVS table <table>,  returns zero if NULL
// is passed in for <table>.

cardinal kvs_number_of_buckets(kvs_table_t table) {
    kvs_table_s *this_table = (kvs_table_s *) table;

    if (table != NULL) return this_table->bucket_count;
    else return 0;
    
} // end kvs_number_of_buckets


// ---------------------------------------------------------------------------
// function:  kvs_number_of_entries( table )
// ---------------------------------------------------------------------------
//
// Returns  the number of entries  stored in KVS table <table>,  returns  zero
// if NULL is passed in for <table>.

cardinal kvs_number_of_entries(kvs_table_t table) {
    kvs_table_s *this_table = (kvs_table_s *) table;

    if (table != NULL) return this_table->entry_count;
    else return 0;        
    
} // end kvs_number_of_entries


// ---------------------------------------------------------------------------
// function:  kvs_dispose_table( table, status )
// ---------------------------------------------------------------------------
//
// Disposes of  KVS table object <table>,  deallocating  all its entries.  The
// table and its entries are disposed of  regardless of any references held to
// any values stored in the table.  The status of the operation is passed back
// in <status>,  unless  NULL  was passed in for <status>.

void kvs_dispose_table(kvs_table_t table, kvs_status_t *status) {
    cardinal index;
    kvs_entry prev_entry, this_entry;
    kvs_table_s *this_table = (kvs_table_s *) table;

    // table must not be NULL
    if (table == NULL) {
        _kvs_set_status(status, KVS_STATUS_INVALID_TABLE);
        return;
    } // end if
    
    if (this_table->entry_count != 0) {
        
        for (index = 0; index < this_table->entry_count; index++) {
        
            this_entry = this_table->bucket[index];
            
            while (this_entry != NULL) {
                prev_entry = this_entry;
                this_entry = this_entry->next;
                _kvs_dispose_entry(prev_entry, NULL);
            } // end while
            
            this_table->bucket[index] = NULL;
            
        } // end for
        
    } // end if
    
    // dispose table base
    DEALLOCATE(this_table);
    
    // set status
    _kvs_set_status(status, KVS_STATUS_SUCCESS);
    
    return;
} // end kvs_dispose_table


// ===========================================================================
// P R I V A T E   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================


// ---------------------------------------------------------------------------
// private function:  _kvs_find_entry( table, key, status )
// ---------------------------------------------------------------------------
//
// If an entry for <key> exists in <table> then the function returns a pointer
// to the entry,  otherwise  it returns NULL.  If the entry is found,  then it
// will be cached within the table,  and a subsequent request to find the same
// same entry  will then return the  cached entry pointer,  which is  slightly
// faster than a lookup of a  non-cached  entry.  The  reference count  of the
// entry is  not  incremented by this function.  The  status  of the operation
// is passed back in <status>,  unless NULL was passed in for <status>.

static kvs_entry _kvs_find_entry(kvs_table_t table,
                                   kvs_key_t key,
                                kvs_status_t *status) {
    cardinal index;
    kvs_entry this_entry;
    kvs_table_s *this_table = (kvs_table_s *) table;
    
    // check if the entry has been cached
    if ((this_table->last_retrieved_entry != NULL) &&
        (key == this_table->last_retrieved_entry->key)) {
        
        // entry has been cached by previous lookup
        
        // set status
        _kvs_set_status(status, KVS_STATUS_SUCCESS);
        
        // return pointer to the cached entry
        return this_table->last_retrieved_entry;
    } // end if
    
    // calculate the bucket index for key
    index = key % this_table->bucket_count;

    if /* bucket is empty */ (this_table->bucket[index] == NULL) {
        
        // set status
        _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND);
        
        // return null, indicating entry not found
        return NULL;
    }
    else /* bucket not empty */ {
    
        // first entry in this bucket is starting point
        this_entry = this_table->bucket[index];
        
        // check every entry in this bucket for a key match
        while ((this_entry->key != key) && (this_entry->next != NULL))
            this_entry = this_entry->next;

        // the entry is found if there was a key match

        if /* key matched */ (this_entry->key == key) {
            
            // cache the entry for faster subsequent lookup
            this_table->last_retrieved_entry = this_entry;
            
            // set status
            _kvs_set_status(status, KVS_STATUS_SUCCESS);
            
            // return pointer to entry found
            return this_entry;
        }
        else /* key did not match */ {
            
            // entry not found
            
            // set status
            _kvs_set_status(status, KVS_STATUS_ENTRY_NOT_FOUND);
            
            // return null, indicating entry not found
            return NULL;
        } // end if
    } // end if
} // end _kvs_find_entry


// ---------------------------------------------------------------------------
// private function:  _kvs_calc_null_terminated_data_size( data )
// ---------------------------------------------------------------------------
//
// Returns the size of null-terminated data <data>.

static fmacro cardinal _kvs_calc_null_terminated_data_size(kvs_data_t data) {
    octet_t *_data = (octet_t *) &data;
    cardinal index = 0;
    
    while (_data[index] != 0) {
        if (index < KVS_MAX_STRING_SIZE)
            index++;
        else
            return 0;
    } // end while
    
    return index;
} // end _kvs_calc_null_terminated_data_size


// ---------------------------------------------------------------------------
// private function:  _kvs_new_entry_with_copy( key, value, size, nt, status )
// ---------------------------------------------------------------------------
//
// Allocates and returns a new table entry initalised with a copy of the data
// at address <value>.  The  status  of  the  operation  is  passed  back  in
// <status>,  unless  NULL  was passed in for <status>.
//
// pre-conditions:
//
//  This function DOES NOT CHECK pre-conditions.  The caller MUST ENFORCE pre-
//  conditions before calling this function:
// 
//  o  key must never be 0
//  o  size must never be 0
//  o  value must never be NULL
//
// post-conditions:
//
//  o  a new entry has been allocated
//  o  additional space for <size> bytes has been allocated
//  o  entry's key field contains key
//  o  entry's value field contains pointer to newly allocated copy of
//     <size> number of bytes starting at address <value>
//  o  entry's size field contains size
//  o  entry's null_terminated flas contains null_terminated
//  o  entry's pending_removal flag contains false
//  o  entry's next field contains NULL
//  o  KVS_STATUS_SUCCESS is passed back in <status>
//  o  pointer to entry is returned
//
// error-conditions:
//
//  if memory allocation fails:
//  o  any partial allocation is rolled back
//  o  KVS_STATUS_ALLOCATION_FAILED is passed back in <status>
//  o  NULL is returned

static fmacro kvs_entry _kvs_new_entry_with_copy(kvs_key_t key,
                                                kvs_data_t value,
                                                  cardinal size,
                                                 bool null_terminated,
                                              kvs_status_t *status) {
    kvs_entry_s *new_entry;
    octet_t *source, *target;
    cardinal index;

    // allocate storage for new entry
    new_entry = ALLOCATE(sizeof(kvs_entry_s));
    
    // exit if allocation failed
    if (new_entry == NULL) {
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // store key
    new_entry->key = key;

    // initialise meta data
    new_entry->size = size;
    new_entry->next = NULL;
    new_entry->ref_count = 1;
    new_entry->null_terminated = null_terminated;
    new_entry->marked_for_removal = false;
            
    // allocate storage for a copy of the data
    new_entry->value = ALLOCATE(size);
        
    // exit if allocation failed
    if (new_entry->value == NULL) {
        
        // undo allocation for entry
        DEALLOCATE(new_entry);
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // copy data
    index = 0;
    source = value;
    target = new_entry->value;
    while (index <= size) {
        target[index] = source[index];
        index++;
    } // end for
        
    // set status and return
    _kvs_set_status(status, KVS_STATUS_SUCCESS);
    return new_entry;
} // end _kvs_new_entry_with_copy


// ---------------------------------------------------------------------------
// private function:  _kvs_new_entry_with_ref( key, value, size, nt, status )
// ---------------------------------------------------------------------------
//
// Allocates and returns a new table entry  initalised with a reference to the
// data at address <value>.  The  status of the operation  is  passed back  in
// <status>,  unless  NULL  was passed in for <status>.
//
// pre-conditions:
//
//  This function DOES NOT CHECK pre-conditions.  The caller MUST ENFORCE pre-
//  conditions before calling this function:
// 
//  o  key must never be 0
//  o  size must never be 0
//  o  value must never be NULL
//
// post-conditions:
//
//  o  a new entry has been allocated
//  o  entry's key field contains key
//  o  entry's value field contains <value>
//  o  entry's size field contains size
//  o  entry's null_terminated flas contains null_terminated
//  o  entry's pending_removal flag contains false
//  o  entry's next field contains NULL
//  o  KVS_STATUS_SUCCESS is passed back in <status>
//  o  pointer to entry is returned
//
// error-conditions:
//
//  if memory allocation fails:
//  o  KVS_STATUS_ALLOCATION_FAILED is passed back in <status>
//  o  NULL is returned

static fmacro kvs_entry _kvs_new_entry_with_ref(kvs_key_t key,
                                               kvs_data_t value,
                                                 cardinal size,
                                                bool null_terminated,
                                             kvs_status_t *status) {
    kvs_entry_s *new_entry;
    
    // allocate storage for new entry
    new_entry = ALLOCATE(sizeof(kvs_entry_s));
    
    // exit if allocation failed
    if (new_entry == NULL) {
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // store key and value
    new_entry->key = key;
    new_entry->value = value;
    
    // initialise meta data
    new_entry->size = size;
    new_entry->next = NULL;
    new_entry->ref_count = 1;
    new_entry->null_terminated = null_terminated;
    new_entry->marked_for_removal = false;
        
    // set status and return
    _kvs_set_status(status, KVS_STATUS_SUCCESS);
    return new_entry;
} // end _kvs_new_entry_with_ref


static fmacro kvs_data_t _kvs_retrieve_copy(kvs_entry_s *entry,
                                           kvs_status_t *status) {
    kvs_entry_s *new_entry;
    octet_t *source, *new_copy;
    cardinal index, size;
    
    // allocate storage for a copy of the data
    new_copy = ALLOCATE(entry->size);
    
    // exit if allocation failed
    if (new_copy == NULL) {
        
        // set status and return NULL
        _kvs_set_status(status, KVS_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // copy data
    index = 0;
    size = entry->size;
    source = new_entry->value;
    while (index <= size) {
        new_copy[index] = source[index];
        index++;
    } // end for
    
    // set status and return
    _kvs_set_status(status, KVS_STATUS_SUCCESS);
    return (kvs_data_t) new_copy;
} // end _kvs_retrieve_copy


// ---------------------------------------------------------------------------
// private function:  _kvs_dispose_entry( entry, status )
// ---------------------------------------------------------------------------
//
// Deallocates <entry>. Status passed back in <status> unless NULL passed in.

static fmacro void _kvs_dispose_entry(kvs_entry entry, kvs_status_t *status) {

    // don't try to free any null pointers for entry
    if (entry == NULL) {
    
        // set status
        _kvs_set_status(status, KVS_STATUS_INVALID_ENTRY);
        
        // exit
        return;
    } // end if
    
    // don't try to free any null pointers for value either, just in case
    if (entry->value != NULL) {
    
        // deallocate entry's value
        DEALLOCATE(entry->value);
        
        // set status
        _kvs_set_status(status, KVS_STATUS_SUCCESS);
        
        // don't exit here, the entry itself still needs to be deallocated
    } // end if
    
    // deallocate the entry itself
    DEALLOCATE(entry);
    
    return;
} // end _kvs_dispose_entry


// END OF FILE
