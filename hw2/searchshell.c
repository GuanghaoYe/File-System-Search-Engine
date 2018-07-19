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

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "memindex.h"
#include "filecrawler.h"
#include "fileparser.h"

static void Usage(void);

void ProcessQuery(DocTable, MemIndex);
size_t ParseQuery(char*, char*[]);

#define BUFSIZE (128)

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3)
    Usage();
  if (argc == 3 && strcmp(argv[2], "-s") != 0) {
    Usage();
  }
  SetFilter(argc == 3);
  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - crawl from a directory provided by argv[1] to produce and index
  //  - prompt the user for a query and read the query from stdin, in a loop
  //  - split a query into words (check out strtok_r)
  //  - process a query against the index and print out the retlists
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  DocTable doctable;
  MemIndex index;
  int retval;
  printf("Indexing \'%s\'\n", argv[1]);
  retval = CrawlFileTree(argv[1], &doctable, &index);
  if (retval != 1) {
    Usage();
    return EXIT_FAILURE;
  }
  ProcessQuery(doctable, index);
  printf("Shut down...\n");
  FreeMemIndex(index);
  FreeDocTable(doctable);
  return EXIT_SUCCESS;
}

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot> [-s]\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n" \
          "[-s] is an optional flag for remove the stop words\n");
  exit(EXIT_FAILURE);
}


size_t ParseQuery(char* buf, char* query[]) {
  char* token;
  char *ptr = buf;
  char* rest;
  size_t len = 0;
  buf[strlen(buf)-1] = '\0';  // remove the newline character
  while ((token = strtok_r(ptr, " ", &rest))) {
    if (Unfiltered(token))
      query[len++] = token;
    ptr = rest;
  }
  return len;
}

void ProcessQuery(DocTable table, MemIndex index) {
  LinkedList retlist;
  char buf[BUFSIZE];
  char* query[BUFSIZE];
  puts("enter query:");
  while (fgets(buf, BUFSIZE, stdin) != NULL) {
    size_t len = ParseQuery(buf, query);
    retlist  = MIProcessQuery(index, query, len);
    if (retlist == NULL || NumElementsInLinkedList(retlist) == 0) {
      puts("enter query:");
      continue;
    }
    LLIter llit = LLMakeIterator(retlist, 0);
    Verify333(llit != NULL);
    do {
      SearchResult* sr;
      LLIteratorGetPayload(llit, (LLPayload_t*)&sr);
      char* filename =  DTLookupDocID(table, sr->docid);
      printf("%s (%d)\n", filename, sr->rank);
    } while (LLIteratorDelete(llit, (LLPayloadFreeFnPtr)free));
    FreeLinkedList(retlist, (LLPayloadFreeFnPtr)free);
    LLIteratorFree(llit);
    puts("enter query:");
  }
}

