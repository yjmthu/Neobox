#include <httplib.h>
#include <cstdlib>


namespace httplib {

  HttpBase::HttpBase()
  : res(nullptr) {

  }

  HttpBase::~HttpBase() {
    delete res;
  }

  bool HttpBase::ParseUrl(const std::string& url) {
    size_t first, last;
    if (url.find("http://") == 0) {
      first = 7;
      req.port = 80;
    } else if (url.find("https://") == 0) {
      first = 8;
      req.port = 443;
    } else {
      return false;
    }

    last = url.find('/', first);
    if (last == std::string::npos) {
      return false;
    }
    req.domain = url.substr(first, last-first);

    first = req.domain.find(':', first);
    if (first != std::string::npos) {
      std::string port = req.domain.substr(first+1);
      req.port = std::atoi(port.c_str());
      req.domain.erase(first);
    }

    req.path = url.substr(last);

    return true;
  }

  void HttpBase::BuildPost(std::ostream& stream, const std::string& data) {
    stream << "POST " << req.path << " HTTP/1.1\r\n";
    stream << "Host: " << req.domain << "\r\n";
    stream << "Accept: */*\r\n";
    stream << "Content-Type:application/x-www-form-urlencoded\r\n";
    stream << "Content-Length: " << data.length() << "\r\n";
    stream << "Connection: close\r\n\r\n";
    stream << data;
  }

  void HttpBase::BuildGet(std::ostream& stream) {
    stream << "GET " << req.path << " HTTP/1.1\r\n";
    stream << "Host: " << req.domain << "\r\n";
    stream << "Accept: */*\r\n";
    stream << "Connection: close\r\n\r\n";
  }

  void HttpBase::ReadResponse(boost::asio::streambuf& response) {
    std::istream istr(&response);
    res = new HttpResponse;
    istr >> res->httpVersion;
    istr >> res->status;
    getline(istr, res->statusMessage);

    if (!istr || res->httpVersion.substr(0, 5) != "HTTP/") {
      printf("响应无效\n");
    }

    boost::asio::read_until(socket, response, "\r\n\r\n");

    std::string header;
    int len = 0;
    while (std::getline(istr, header) && header != "\r") {
      if (header.find("Content-Length: ") == 0) {
        std::stringstream stream;
        stream << header.substr(16);
        stream >> len;
      }
    }
  
    long size = response.size();
 
    if (size > 0) {
      // ... do nothing ...
    }
    
    boost::system::error_code error;
    
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
      size = response.size();
      if (len != 0) {
        std::cout << size << "  Byte  " << (size * 100) / len << "%" << std::endl;
      }
    }
 
    if (error != boost::asio::error::eof) {
      throw boost::system::system_error(error);
    }

    std::istream is(&response);
    is.unsetf(std::ios_base::skipws);
    res->body.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
  }

  HttpGet::HttpGet() {
  }

  const HttpResponse* HttpGet::Get(const std::string& url) {
    using boost::asio::ip::tcp;
    if (res) {
      delete res;
      res = nullptr;
    }

    boost::asio::io_service ioService;
  
    tcp::resolver resolver(ioService);
    tcp::resolver::query query(req.domain, "http");
    tcp::resolver::iterator iter = resolver.resolve(query);
    
    tcp::socket socket(ioService);
    boost::asio::connect(socket, iter);

    ParseUrl(url);
    boost::asio::streambuf request;
    std::ostream ostr(&request);
    BuildGet(ostr);

    boost::asio::streambuf response;
    boost::asio::write(socket, request);
    boost::asio::read_until(socket, response, "\r\n");

    ReadResponse(response);

    return res;
  }

  HttpPost::HttpPost(const std::string& body) {
    req.body = body;
  }

  const HttpResponse* HttpPost::Post(const std::string& url) {
    using boost::asio::ip::tcp;
    if (res) {
      delete res;
      res = nullptr;
    }

    boost::asio::io_service ioService;
  
    tcp::resolver resolver(ioService);
    tcp::resolver::query query(req.domain, "http");
    tcp::resolver::iterator iter = resolver.resolve(query);
    
    tcp::socket socket(ioService);
    boost::asio::connect(socket, iter);

    ParseUrl(url);
    boost::asio::streambuf request;
    std::ostream ostr(&request);
    BuildPost(ostr, req.body);

    boost::asio::streambuf response;
    boost::asio::write(socket, request);
    boost::asio::read_until(socket, response, "\r\n");

    ReadResponse(response);

    return res;
  }

}
