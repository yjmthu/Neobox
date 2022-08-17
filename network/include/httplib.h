#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <iostream>
#include <map>
#include <string>

#include <boost/asio.hpp>
#include <boost/system.hpp>

namespace httplib
{

struct HttpRequest
{
    int port;
    std::string domain;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
};

struct HttpResponse
{
    int status;
    std::string httpVersion;
    std::string statusMessage;
    std::string location;
    std::string body;
    std::map<std::string, std::string> headers;
};

class HttpBase
{
  public:
    HttpBase();
    ~HttpBase();
    bool SetFollowLocation(bool follow);

  protected:
    HttpRequest req;
    HttpResponse *res;

    bool ParseUrl(const std::string &url);
    void BuildPost(std::ostream &stream, const std::string &data);

    template <typename _TcpTy> void WriteRequest(_TcpTy &socket);
    template <typename _TcpTy> void ReadResponse(_TcpTy &socket);

    // virtual void handle_connect(const boost::system::error_code& error) = 0;
};

class HttpGet : public HttpBase
{
  public:
    HttpGet();
    // ~HttpGet();
    const HttpResponse *Get(const std::string &url);
    inline const HttpResponse *Get(const std::u8string &url)
    {
        return Get(std::string(url.begin(), url.end()));
    }

  private:
    int timeOut = 3000;
    void GetHttp();
    void GetHttps();
};

class HttpPost : public HttpBase
{
  public:
    HttpPost(const std::string &body);
    // ~HttpPost();
    const HttpResponse *Post(const std::string &url);
    inline const HttpResponse *Post(const std::u8string &url)
    {
        return Post(std::string(url.begin(), url.end()));
    }

  private:
};

} // namespace httplib

#endif
