/*
 * Copyright ©2018 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Summer Quarter 2018 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

extern "C" {
  #include "./CSE333.h"
  #include "./HashTable.h"
  #include "./HashTable_priv.h"
  #include "./LinkedList.h"
  #include "./LinkedList_priv.h"
}
#include "./test_suite.h"
#include "./test_hashtable.h"

namespace hw1 {

// our payload structure

typedef struct {
  int num;
} ExampleValue, *ExampleValuePtr;


void ExampleValueFree(HTValue_t value) {
  Verify333(value != NULL);
  free(value);
}

TEST_F(Test_HashTable, HTSTestEmptyTable) {
  HashTable ht = AllocateHashTable(0);
}

TEST_F(Test_HashTable, HTSTestEmptyTableIterator) {
  HashTable ht = AllocateHashTable(1);
  HTIter iter;
  HTKeyValue kv,old_kv;
  iter = HashTableMakeIterator(ht);
  HTIteratorDelete(iter, &old_kv);
  iter = HashTableMakeIterator(ht);
  HTIteratorFree(iter);

  // free the hash table
  FreeHashTable(ht, &ExampleValueFree);
}

TEST_F(Test_HashTable, HTSTestFNV) {
  ExampleValuePtr evp;
  HTIter iter;
  HTKeyValue kv, old_kv;
  HTKey_t i;
  HashTable ht = AllocateHashTable(1000);
  Verify333(ht != NULL);

  // insert 20,000 elements (load factor = 2.0)
  for (i = 0; i < 200000; i++) {
    evp = (ExampleValuePtr) malloc(sizeof(ExampleValue));
    Verify333(evp != NULL);
    evp->num = i;

    // make sure HT has the right # of elements in it to start
    ASSERT_EQ(NumElementsInHashTable(ht), (HWSize_t) i);

    // insert a new element
    kv.key = FNVHashInt64((HTValue_t)i);
    kv.value = (HTValue_t)evp;
    ASSERT_EQ(InsertHashTable(ht, kv, &old_kv), 1);

    // make sure hash table has right # of elements post-insert
    ASSERT_EQ(NumElementsInHashTable(ht), (HWSize_t) (i+1));
  }
  ASSERT_EQ(LookupHashTable(ht, FNVHashInt64((HTValue_t)100), &kv), 1);
  ASSERT_EQ(kv.key, FNVHashInt64((HTValue_t)100));
  ASSERT_EQ(((ExampleValuePtr) kv.value)->num, 100);

  ASSERT_EQ(LookupHashTable(ht, FNVHashInt64((HTValue_t)18583), &kv), 1);
  ASSERT_EQ(kv.key, FNVHashInt64((HTValue_t)18583));
  ASSERT_EQ(((ExampleValuePtr) kv.value)->num, 18583);

  // make sure non-existent value cannot be found
  ASSERT_EQ(LookupHashTable(ht, FNVHashInt64((HTValue_t)200000), &kv), 0);

  // delete a value
  ASSERT_EQ(RemoveFromHashTable(ht, FNVHashInt64((HTValue_t)100), &kv), 1);
  ASSERT_EQ(kv.key, FNVHashInt64((HTValue_t)100));
  ASSERT_EQ(((ExampleValuePtr) kv.value)->num, 100);
  ExampleValueFree(kv.value);   // since we malloc'ed it, we must free it

  // make sure it's deleted
  ASSERT_EQ(LookupHashTable(ht, FNVHashInt64((HTValue_t)100), &kv), 0);
  ASSERT_EQ(NumElementsInHashTable(ht), (HWSize_t) 199999);
  i = 0;
  iter = HashTableMakeIterator(ht);
  Verify333(iter != NULL);

  while (HTIteratorPastEnd(iter) == 0) {
    ASSERT_EQ(HTIteratorGet(iter, &kv), 1);
    Verify333(kv.key != FNVHashInt64((HTValue_t)100));   // we deleted it

    // advance the iterator
    HTIteratorNext(iter);
    i++;
  }
  ASSERT_EQ(i, 199999);

  // free the iterator
  HTIteratorFree(iter);

  // free the hash table
  FreeHashTable(ht, &ExampleValueFree);
}



}  // namespace hw1
