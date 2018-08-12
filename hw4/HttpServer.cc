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

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "./FileReader.h"
#include "./HttpConnection.h"
#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpServer.h"
#include "./libhw3/QueryProcessor.h"

using std::cerr;
using std::cout;
using std::endl;

namespace hw4 {

// This is the function that threads are dispatched into
// in order to process new client connections.
void HttpServer_ThrFn(ThreadPool::Task *t);

// Given a request, produce a response.
HttpResponse ProcessRequest(const HttpRequest &req,
                            const std::string &basedir,
                            const std::list<std::string> *indices);

// Process a file request.
HttpResponse ProcessFileRequest(const std::string &uri,
                                const std::string &basedir);

// Process a query request.
HttpResponse ProcessQueryRequest(const std::string &uri,
                                 const std::list<std::string> *indices);

bool HttpServer::Run(void) {
  // Create the server listening socket.
  int listen_fd;
  cout << "  creating and binding the listening socket..." << endl;
  if (!ss_.BindAndListen(AF_INET6, &listen_fd)) {
    cerr << endl << "Couldn't bind to the listening socket." << endl;
    return false;
  }

  // Spin, accepting connections and dispatching them.  Use a
  // threadpool to dispatch connections into their own thread.
  cout << "  accepting connections..." << endl << endl;
  ThreadPool tp(kNumThreads);
  while (1) {
    HttpServerTask *hst = new HttpServerTask(HttpServer_ThrFn);
    hst->basedir = staticfile_dirpath_;
    hst->indices = &indices_;
    if (!ss_.Accept(&hst->client_fd,
                    &hst->caddr,
                    &hst->cport,
                    &hst->cdns,
                    &hst->saddr,
                    &hst->sdns)) {
      // The accept failed for some reason, so quit out of the server.
      // (Will happen when kill command is used to shut down the server.)
      break;
    }
    // The accept succeeded; dispatch it.
    tp.Dispatch(hst);
  }
  return true;
}

void HttpServer_ThrFn(ThreadPool::Task *t) {
  // Cast back our HttpServerTask structure with all of our new
  // client's information in it.
  std::unique_ptr<HttpServerTask> hst(static_cast<HttpServerTask *>(t));
  cout << "  client " << hst->cdns << ":" << hst->cport << " "
       << "(IP address " << hst->caddr << ")" << " connected." << endl;

  bool done = false;
  HttpConnection connection(hst->client_fd);
  while (!done) {
    // Use the HttpConnection class to read in the next request from
    // this client, process it by invoking ProcessRequest(), and then
    // use the HttpConnection class to write the response.  If the
    // client sent a "Connection: close\r\n" header, then shut down
    // the connection.

    // MISSING:
    HttpRequest request;
    if(!connection.GetNextRequest(&request)) {
      close(hst->client_fd);
      done = true;
    }
    if (request.headers["Connection"] == "close") {
      close(hst->client_fd);
      done = true;
    }
    HttpResponse response =  ProcessRequest(request, hst->basedir, hst->indices);
    if(!connection.WriteResponse(response)) {
      done =true;
      close(hst->client_fd);
    }
  }
}

HttpResponse ProcessRequest(const HttpRequest &req,
                            const std::string &basedir,
                            const std::list<std::string> *indices) {
  // Is the user asking for a static file?
  if (req.URI.substr(0, 8) == "/static/") {
    return ProcessFileRequest(req.URI, basedir);
  }

  // The user must be asking for a query.
  return ProcessQueryRequest(req.URI, indices);
}


std::string getContentType(std::string suffix) {
  map<string, string> suffixMap = {
    {"ac3","audio/ac3"},
    {"amr","audio/AMR"},
    {"awb","audio/AMR-WB"},
    {"acn","audio/asc"},
    {"atx","audio/ATRAC-X"},
    {"at3","audio/ATRAC3"},
    {"aa3","audio/ATRAC3"},
    {"omg","audio/ATRAC3"},
    {"au","audio/basic"},
    {"snd","audio/basic"},
    {"dls","audio/dls"},
    {"evc","audio/EVRC"},
    {"evb","audio/EVRCB"},
    {"enw","audio/EVRCNW"},
    {"evw","audio/EVRCWB"},
    {"lbc","audio/iLBC"},
    {"l16","audio/L16"},
    {"m4a","audio/mp4"},
    {"mp3","audio/mpeg"},
    {"mpga","audio/mpeg"},
    {"mp1","audio/mpeg"},
    {"mp2","audio/mpeg"},
    {"oga","audio/ogg"},
    {"ogg","audio/ogg"},
    {"opus","audio/ogg"},
    {"spx","audio/ogg"},
    {"sid","audio/prs.sid"},
    {"psid","audio/prs.sid"},
    {"qcp","audio/qcelp"},
    {"smv","audio/SMV"},
    {"dts","audio/vnd.dts"},
    {"rip","audio/vnd.rip"},
    {"otf","font/otf"},
    {"ttf","font/ttf"},
    {"woff","font/woff"},
    {"woff2","font/woff2"},
    {"bmp","image/bmp"},
    {"dib","image/bmp"},
    {"cgm","image/cgm"},
    {"emf","image/emf"},
    {"fits","image/fits"},
    {"fit","image/fits"},
    {"fts","image/fits"},
    {"gif","image/gif"},
    {"ief","image/ief"},
    {"jls","image/jls"},
    {"jp2","image/jp2"},
    {"jpg2","image/jp2"},
    {"jpg","image/jpeg"},
    {"jpeg","image/jpeg"},
    {"jpe","image/jpeg"},
    {"jfif","image/jpeg"},
    {"jpm","image/jpm"},
    {"jpgm","image/jpm"},
    {"jpx","image/jpx"},
    {"jpf","image/jpx"},
    {"ktx","image/ktx"},
    {"png","image/png"},
    {"pti","image/prs.pti"},
    {"svg","image/svg+xml"},
    {"svgz","image/svg+xml"},
    {"t38","image/t38"},
    {"tiff","image/tiff"},
    {"tif","image/tiff"},
    {"tfx","image/tiff-fx"},
    {"dwg","image/vnd.dwg"},
    {"dxf","image/vnd.dxf"},
    {"fpx","image/vnd.fpx"},
    {"fst","image/vnd.fst"},
    {"wmf","image/wmf"},
    {"igs","model/iges"},
    {"iges","model/iges"},
    {"msh","model/mesh"},
    {"mesh","model/mesh"},
    {"silo","model/mesh"},
    {"dwf","model/vnd.dwf"},
    {"gdl","model/vnd.gdl"},
    {"gsm","model/vnd.gdl"},
    {"win","model/vnd.gdl"},
    {"dor","model/vnd.gdl"},
    {"lmp","model/vnd.gdl"},
    {"rsm","model/vnd.gdl"},
    {"msm","model/vnd.gdl"},
    {"ism","model/vnd.gdl"},
    {"gtw","model/vnd.gtw"},
    {"mts","model/vnd.mts"},
    {"vtu","model/vnd.vtu"},
    {"wrl","model/vrml"},
    {"vrml","model/vrml"},
    {"x3db","model/x3d+xml"},
    {"ics","text/calendar"},
    {"ifb","text/calendar"},
    {"css","text/css"},
    {"csv","text/csv"},
    {"soa","text/dns"},
    {"zone","text/dns"},
    {"html","text/html"},
    {"htm","text/html"},
    {"cnd","text/jcr-cnd"},
    {"markdown","text/markdown"},
    {"md","text/markdown"},
    {"miz","text/mizar"},
    {"n3","text/n3"},
    {"txt","text/plain"},
    {"asc","text/plain"},
    {"text","text/plain"},
    {"pm","text/plain"},
    {"el","text/plain"},
    {"c","text/plain"},
    {"h","text/plain"},
    {"cc","text/plain"},
    {"hh","text/plain"},
    {"cxx","text/plain"},
    {"hxx","text/plain"},
    {"f90","text/plain"},
    {"conf","text/plain"},
    {"log","text/plain"},
    {"rtx","text/richtext"},
    {"sgml","text/sgml"},
    {"sgm","text/sgml"},
    {"t","text/troff"},
    {"tr","text/troff"},
    {"roff","text/troff"},
    {"ttl","text/turtle"},
    {"uris","text/uri-list"},
    {"uri","text/uri-list"},
    {"vcf","text/vcard"},
    {"vcard","text/vcard"},
    {"a","text/vnd.a"},
    {"abc","text/vnd.abc"},
    {"fly","text/vnd.fly"},
    {"xml","text/xml"},
    {"xsd","text/xml"},
    {"rng","text/xml"},
    {"3gp","video/3gpp"},
    {"3gpp","video/3gpp"},
    {"3g2","video/3gpp2"},
    {"3gpp2","video/3gpp2"},
    {"mj2","video/mj2"},
    {"mjp2","video/mj2"},
    {"mp4","video/mp4"},
    {"mpg4","video/mp4"},
    {"m4v","video/mp4"},
    {"mpeg","video/mpeg"},
    {"mpg","video/mpeg"},
    {"mpe","video/mpeg"},
    {"m1v","video/mpeg"},
    {"m2v","video/mpeg"},
    {"ogv","video/ogg"},
    {"fvt","video/vnd.fvt"},
    {"mid","audio/midi"},
    {"midi","audio/midi"},
    {"kar","audio/midi"},
    {"aif","audio/x-aiff"},
    {"aiff","audio/x-aiff"},
    {"aifc","audio/x-aiff"},
    {"flac","audio/x-flac"},
    {"mod","audio/x-mod"},
    {"ult","audio/x-mod"},
    {"uni","audio/x-mod"},
    {"m15","audio/x-mod"},
    {"mtm","audio/x-mod"},
    {"669","audio/x-mod"},
    {"med","audio/x-mod"},
    {"s3m","audio/x-s3m"},
    {"stm","audio/x-stm"},
    {"wav","audio/x-wav"},
    {"webp","image/webp"},
    {"rgb","image/x-rgb"},
    {"tga","image/x-targa"},
    {"pod","text/x-pod"},
    {"etx","text/x-setext"},
    {"webm","video/webm"},
    {"flv","video/x-flv"},
  };
  return suffixMap[suffix];
}

HttpResponse ProcessFileRequest(const std::string &uri,
                                const std::string &basedir) {
  // The response we'll build up.
  HttpResponse ret;

  // Steps to follow:
  //  - use the URLParser class to figure out what filename
  //    the user is asking for.
  //
  //  - use the FileReader class to read the file into memory
  //
  //  - copy the file content into the ret.body
  //
  //  - depending on the file name suffix, set the response
  //    Content-type header as appropriate, e.g.,:
  //      --> for ".html" or ".htm", set to "text/html"
  //      --> for ".jpeg" or ".jpg", set to "image/jpeg"
  //      --> for ".png", set to "image/png"
  //      etc.
  //
  // be sure to set the response code, protocol, and message
  // in the HttpResponse as well.
  std::string fname = "";

  // MISSING:
  URLParser parser;
  parser.Parse(uri);
  fname = parser.get_path();
  fname = fname.substr(8);
  FileReader reader(basedir, fname);
  if (reader.ReadFile(&ret.body)) {
    ret.protocol = "HTTP/1.1";
    ret.message = "OK";
    ret.response_code = 200;
    string suffix = fname.substr(fname.rfind("."), fname.length()-1);
    ret.headers["content-type"] = getContentType(suffix);
    return ret;
  }
  // If you couldn't find the file, return an HTTP 404 error.
  ret.protocol = "HTTP/1.1";
  ret.response_code = 404;
  ret.message = "Not Found";
  ret.body = "<html><body>Couldn't find file \"";
  ret.body +=  EscapeHTML(fname);
  ret.body += "\"</body></html>";
  return ret;
}

std::string getItemHtml(hw3::QueryProcessor::QueryResult result) {
  char buf[1024];
  snprintf(buf, sizeof(buf), 
    "<li> <a href=\"/static/%s\">%s</a> [%d]<br></li>\n",
    result.document_name,
    result.document_name,
    result.rank);
  return std::string(buf);
}

HttpResponse ProcessQueryRequest(const std::string &uri,
                                 const std::list<std::string> *indices) {
  // The response we're building up.
  HttpResponse ret;

  // Your job here is to figure out how to present the user with
  // the same query interface as our solution_binaries/http333d server.
  // A couple of notes:
  //
  //  - no matter what, you need to present the 333gle logo and the
  //    search box/button
  //
  //  - if the user had previously typed in a search query, you also
  //    need to display the search results.
  //
  //  - you'll want to use the URLParser to parse the uri and extract
  //    search terms from a typed-in search query.  convert them
  //    to lower case.
  //
  //  - you'll want to create and use a hw3::QueryProcessor to process
  //    the query against the search indices
  //
  //  - in your generated search results, see if you can figure out
  //    how to hyperlink results to the file contents, like we did
  //    in our solution_binaries/http333d.

  // MISSING:
  string logoHtml = "<html><head><title>333gle</title></head>\n"\
            "<body>\n"\
            "<center style=\"font-size:500%;\">\n"\
            "<span style=\"position:relative;bottom:-0.33em;color:orange;\">3"\
            "</span><span style=\"color:red;\">3</span><span style=\"color:"\
            "gold;\">3</span><span style=\"color:blue;\">g</span><span style"\
            "=\"color:green;\">l</span><span style=\"color:red;\">e</span>\n"\
            "</center>\n"\
            "<p>\n"\
            "<div style=\"height:20px;\"></div>\n"\
            "<center>\n"\
            "<form action=\"/query\" method=\"get\">\n"\
            "<input type=\"text\" size=30 name=\"terms\" />\n"\
            "<input type=\"submit\" value=\"Search\" />\n"\
            "</form>\n"\
            "</center><p>\n";
  ret.body += logoHtml;
  URLParser parser;
  parser.Parse(uri);
  std::string query = parser.get_args()["terms"];
  boost::trim(query);
  boost::to_lower(query);
  vector<string> tokens;
  boost::split(tokens, query, boost::is_any_of(" "),boost::token_compress_on);
  hw3::QueryProcessor processor(*indices);
  vector<hw3::QueryProcessor::QueryResult> results =
    processor.ProcessQuery(tokens);
  ret.body += "<p><br>" + std::to_string(results.size()) + "results found for<b>" +
              query + "</b> </p> + <p></p>\n<ul>";
  for(auto result: results) {
    ret.body += getItemHtml(result);
  }
  ret.body += "</ul></body>\n" + std::string("</html>\n");
  return ret;
}

}  // namespace hw4
