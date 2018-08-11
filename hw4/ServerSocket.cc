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

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int *listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd".

  // MISSING:
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = ai_family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_flags |= AI_V4MAPPED;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  addrinfo *result;

  int res = getaddrinfo(nullptr, std::to_string(port_).c_str(), &hints, &result);

  // Did addrinfo() fail?
  if (res != 0) {
    std::cerr << "getaddrinfo() failed: ";
    std::cerr << gai_strerror(res) << std::endl;
    return -1;
  }

  *listen_fd = -1;
  for (addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
    *listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (*listen_fd == -1) {
      std::cerr << "socket() failed: " << strerror(errno) << std::endl;
      *listen_fd = -1;
      continue;
    }
    
    // set socket reuse address
    int optval = 1;
    setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // try to bind
    if (!bind(*listen_fd, rp->ai_addr, rp->ai_addrlen)) {
      sock_family_ = rp->ai_family;
      break;
    }
    close(*listen_fd);
    *listen_fd = -1;
  }
  if (*listen_fd <= 0) {
    return false;
  }
  if (listen(*listen_fd, SOMAXCONN) != 0) {
    std::cerr << "Failed to mark socket as listening: ";
    std::cerr << strerror(errno) << std::endl;
    close(*listen_fd);
    return false;
  }
  listen_sock_fd_ = *listen_fd;
  return true;
}


// private helper function to find addresses
bool GetNetInfo(int fd, sockaddr *addr, size_t addrlen,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dnsname,
                          std::string *server_addr,
                          std::string *server_dnsname) {
  char server_hostname[1024];
  server_hostname[0]='\0';
  if (addr->sa_family == AF_INET) {
    // IPv4
    char astring[INET_ADDRSTRLEN];
    sockaddr_in *in4 = reinterpret_cast<sockaddr_in *>(addr);
    inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
    *client_addr = astring;
    *client_port = ntohs(in4->sin_port);
    // done with client

    char addrbuf[INET_ADDRSTRLEN];
    sockaddr_in srvr;
    socklen_t srvrlen = sizeof(srvr);
    getsockname(fd, reinterpret_cast<sockaddr *>(&srvr), &srvrlen);
    inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
    *server_addr = addrbuf;
    getnameinfo(reinterpret_cast<const sockaddr *>(&srvr),
      srvrlen, server_hostname, 1024, nullptr, 0, 0);
    *server_dnsname = server_hostname; 
  } else if (addr->sa_family == AF_INET6) {
    // IPv6
    char astring[INET6_ADDRSTRLEN];
    sockaddr_in6 *in6 = reinterpret_cast<sockaddr_in6 *>(addr);
    inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    *client_addr = astring;
    *client_port = ntohs(in6->sin6_port);
    //done with client

    char addrbuf[INET6_ADDRSTRLEN];
    sockaddr_in6 srvr;
    socklen_t srvrlen = sizeof(srvr);
    getsockname(fd, reinterpret_cast<sockaddr *>(&srvr), &srvrlen);
    inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    *server_addr = addrbuf;
    getnameinfo(reinterpret_cast<const sockaddr *>(&srvr),
      srvrlen, server_hostname, 1024, nullptr, 0, 0);
    *server_dnsname = server_hostname; 
  } else {
    return false;
  }
  char hostname[1024];
  if (getnameinfo(addr, addrlen, hostname, 1024, nullptr, 0, 0) != 0) {
    sprintf(hostname, "[reverse DNS failed]");
    return false;
  }
  *client_dnsname = hostname;
  return true;
}


bool ServerSocket::Accept(int *accepted_fd,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dnsname,
                          std::string *server_addr,
                          std::string *server_dnsname) {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // MISSING:
  sockaddr_storage caddr;
  socklen_t caddr_len = sizeof (caddr);
  do {
    *accepted_fd = accept(listen_sock_fd_,
                          reinterpret_cast<sockaddr *>(&caddr), 
                          &caddr_len);
  } while((*accepted_fd < 0) &&
         ((errno == EINTR) || (errno =EAGAIN) || (errno == EWOULDBLOCK)));
  if (*accepted_fd < 0) {
    std::cerr << "Failure on accept " << strerror(errno) << std::endl;
    return false;
  }
  if(!GetNetInfo(*accepted_fd, 
                 reinterpret_cast<sockaddr*>(&caddr), 
                 caddr_len,
                 client_addr, 
                 client_port,
                 client_dnsname,
                 server_addr,
                 server_dnsname))
    return false;
  return true;
}

}  // namespace hw4
