/* Key Value Storage Library
 *
 *  @file KVS.h
 *  KVS interface
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


#include "../common/common.h"


// ---------------------------------------------------------------------------
// Default KVS table size
// ---------------------------------------------------------------------------

#define KVS_DEFAULT_TABLE_SIZE 20011


// ---------------------------------------------------------------------------
// Maximum size for null-terminated values
// ---------------------------------------------------------------------------

#define KVS_MAX_STRING_SIZE (64*1024) /* 64 KBytes */


// ---------------------------------------------------------------------------
// Opaque key-value table handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t kvs_table_t;


// ---------------------------------------------------------------------------
// Key type
// ---------------------------------------------------------------------------

typedef uint32_t kvs_key_t;


// ---------------------------------------------------------------------------
// Value data pointer type
// ---------------------------------------------------------------------------

typedef void *kvs_data_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef /* kvs_status_t */ enum {
    KVS_STATUS_SUCCESS = 1,
    KVS_STATUS_INVALID_TABLE,
    KVS_STATUS_INVALID_KEY,
    KVS_STATUS_INVALID_DATA,
    KVS_STATUS_INVALID_SIZE,
    KVS_STATUS_KEY_NOT_UNIQUE,
    KVS_STATUS_ALLOCATION_FAILED,
    KVS_STATUS_INVALID_ENTRY,
    KVS_STATUS_ENTRY_NOT_FOUND,
    KVS_STATUS_ENTRY_PENDING_REMOVAL,
    KVS_STATUS_SIZE_OF_ENTRY_UNKNOWN
} kvs_status_t;


// ---------------------------------------------------------------------------
// function:  kvs_new_table( size, status )
// ---------------------------------------------------------------------------
//
// Creates  and returns  a new KVS table object with <size> number of buckets.
// If  zero is passed in <size>,  then  the new table will be created with the
// default table size  as  defined  by  KVS_DEFAULT_TABLE_SIZE.  Returns  NULL
// if the KVS table object could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

kvs_table_t kvs_new_table(cardinal size, kvs_status_t *status);


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
                    kvs_status_t *status);


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
                        kvs_status_t *status);


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
                     kvs_status_t *status);


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
                        kvs_status_t *status);


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
                            kvs_status_t *status);


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
                                kvs_status_t *status);


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
                         kvs_status_t *status);


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
                                        kvs_status_t *status);


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
                                    kvs_status_t *status);


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
                      kvs_status_t *status);


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
                     kvs_status_t *status);


// ---------------------------------------------------------------------------
// function:  kvs_number_of_buckets( table )
// ---------------------------------------------------------------------------
//
// Returns  the number of buckets  of KVS table <table>,  returns zero if NULL
// is passed in for <table>.

cardinal kvs_number_of_buckets(kvs_table_t table);


// ---------------------------------------------------------------------------
// function:  kvs_number_of_entries( table )
// ---------------------------------------------------------------------------
//
// Returns  the number of entries  stored in KVS table <table>,  returns  zero
// if NULL is passed in for <table>.

cardinal kvs_number_of_entries(kvs_table_t table);


// ---------------------------------------------------------------------------
// function:  kvs_dispose_table( table, status )
// ---------------------------------------------------------------------------
//
// Disposes of  KVS table object <table>,  deallocating  all its entries.  The
// table and its entries are disposed of  regardless of any references held to
// any values stored in the table.  The status of the operation is passed back
// in <status>,  unless  NULL  was passed in for <status>.

void kvs_dispose_table(kvs_table_t table, kvs_status_t *status);


// END OF FILE
