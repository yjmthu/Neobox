#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <map>

namespace httplib {

struct HttpRequest {
  std::string domain;
  std::string path;
  std::string body;
  std::map<std::string, std::string> headers;
  int port;
};

struct HttpResponse {
  int status;
  std::string httpVersion;
  std::string statusMessage;
  std::string body;
};

class HttpBase {
public:
  HttpBase();
  ~HttpBase();
  bool SetFollowLocation(bool follow);
protected:
  HttpRequest req;
  HttpResponse *res;
  bool ParseUrl(const std::string& url);
  void BuildGet(std::ostream& stream);
  void BuildPost(std::ostream& stream, const std::string& data);
  void ReadResponse(boost::asio::streambuf& buffer);
};

class HttpGet: public HttpBase {
public:
  HttpGet();
  ~HttpGet();
  const HttpResponse* Get(const std::string& url);
  inline const HttpResponse* Get(const std::u8string& url) {
    return Get(std::string(url.begin(), url.end()));
  }
private:
};

class HttpPost: public HttpBase {
public:
  HttpPost(const std::string& body);
  ~HttpPost();
  const HttpResponse* Post(const std::string& url);
  inline const HttpResponse* Post(const std::u8string& url) {
    return Post(std::string(url.begin(), url.end()));
  }
private:
};

}

#endif

