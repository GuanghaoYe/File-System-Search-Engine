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

#include <iostream>
#include <algorithm>
#include <utitlity>
#include "./QueryProcessor.h"

extern "C" {
  #include "./libhw1/CSE333.h"
}

namespace hw3 {

QueryProcessor::QueryProcessor(list<string> indexlist, bool validate) {
  // Stash away a copy of the index list.
  indexlist_ = indexlist;
  arraylen_ = indexlist_.size();
  Verify333(arraylen_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[arraylen_];
  itr_array_ = new IndexTableReader *[arraylen_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::iterator idx_iterator = indexlist_.begin();
  for (HWSize_t i = 0; i < arraylen_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = new DocTableReader(fir.GetDocTableReader());
    itr_array_[i] = new IndexTableReader(fir.GetIndexTableReader());
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (HWSize_t i = 0; i < arraylen_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

static vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuerySingleIndex(const vector<string> &query,
                                           DocTableReader* doctable,
                                        IndexTableReader* indextable) {
  vector<QueryProcessor::QueryResult> result;
  list<docid_element_header> docidlist;
  DocIDTableReader* docidtable;
  list<DocPositionOffset_t> ret_list;
  docidtable = indextable->LookupWord(query[0]);
  if (docidtable == nullptr)
    return retval;
  docidlist = docidtable->GetDocIDList();
  for (HWSize_t i = 1; i < query.size(); ++i) {
    docidtable = indextable->LookupWord(query[i]);
    if (docidtable == nullptr)
      return retval;
    for(auto it = docidlist.rbegin(); it != docidlist.rend();) {
      bool res = docidtable->LookupDocID(it->docid, &ret_list);
      if (!res) {
        it = docidlist.erase(it);
      } else {
        it->num_positions += ret_list.size();
        it++;
      }
    }
  }
  string filename;
  for(auto it : docidlist) {
    Verify333(doctable->LookupDocID(it.docid, &filename));
    result.push_back({filename, it.num_positions});
  }
  return result;
}


vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) {
  Verify333(query.size() > 0);
  vector<QueryProcessor::QueryResult> finalresult;
  if (query.size() == 0) {
    return finalresult;
  }
  vector<QueryProcessor::QueryResult> result;
  for (HWSize_t i = 0; i < arraylen_; ++i) {
    result = ProcessQuerySingleIndex(query, dtr_array_[i], itr_array_[i]);
    finalresult.insert(finalresult.end(), result.begin(), result.end());
  }

  // Sort the final results.
  std::sort(finalresult.begin(), finalresult.end());
  return finalresult;
}

}  // namespace hw3
