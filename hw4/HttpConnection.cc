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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;

namespace hw4 {

const int BUFSIZE = 1024;

bool HttpConnection::GetNextRequest(HttpRequest *request) {
  // Use "WrappedRead" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header.
  //
  // Once you've seen the request header, use ParseRequest()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes GetNextRequest()!

  // MISSING:
  unsigned char buf[BUFSIZE];
  int len = 1;
  while (len > 0 && (buffer_.find("\r\n\r\n") == std::string::npos)) {
    len = WrappedRead(fd_, buf, BUFSIZE);
    if (len < 0)
      return false;
    buffer_.append(reinterpret_cast<char *>(buf), len);
  }
  int pos = buffer_.find("\r\n\r\n");
  if (pos == std::string::npos) {
    return true;
  } else {
    *request = ParseRequest(pos + 4);
    buffer_ = buffer_.substr(pos + 4);
  }
  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse &response) {
  std::string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         (unsigned char *) str.c_str(),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(size_t end) {
  HttpRequest req;
  req.URI = "/";  // by default, get "/".

  // Get the header.
  std::string str = buffer_.substr(0, end);

  // Split the header into lines.  Extract the URI from the first line
  // and store it in req.URI.  For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers (i.e., req.headers[headername] = headervalue).
  // You should look at HttpResponse.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.

  // MISSING:
  std::vector<string> lines;
  boost::split(lines, str, boost::is_any_of("\r\n"), boost::token_compress_on);
  std::vector<string> tokens;
  boost::split(tokens, lines[0], boost::is_any_of(" "));
  req.URI = tokens[1];
  for (size_t i = 1; i < lines.size(); ++i) {
    if (lines[i].size() == 0)
      continue;
    tokens.clear();
    boost::split(tokens, lines[i], boost::is_any_of(":"));
    for (size_t j = 0; j < tokens.size(); ++j)
      boost::algorithm::trim(tokens[j]);
      boost::to_lower(tokens[0]);
      req.headers[tokens[0]] = tokens[1];
  }


  return req;
}

}  // namespace hw4
