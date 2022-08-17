#include <cstdlib>
#include <httplib.h>

#include <boost/system.hpp>

#include <iostream>
#include <tchar.h>
#include <wincrypt.h>
#include <windows.h>
#include <winhttp.h>

namespace httplib
{

using boost::asio::ip::tcp;

HttpBase::HttpBase() : res(nullptr)
{
}

HttpBase::~HttpBase()
{
    delete res;
}

bool HttpBase::SetFollowLocation(bool follow)
{
    return false;
}

bool HttpBase::ParseUrl(const std::string &url)
{
    size_t first, last;
    if (url.find("http://") == 0)
    {
        first = 7;
        req.port = 80;
    }
    else if (url.find("https://") == 0)
    {
        first = 8;
        req.port = 443;
    }
    else
    {
        return false;
    }

    last = url.find('/', first);
    if (last == std::string::npos)
    {
        req.domain = url.substr(first);
        req.path = "/";
        return true;
    }

    req.domain = url.substr(first, last - first);

    first = req.domain.find(':', first);
    if (first != std::string::npos)
    {
        std::string port = req.domain.substr(first + 1);
        req.port = std::atoi(port.c_str());
        req.domain.erase(first);
    }

    req.path = url.substr(last);

    return true;
}

void HttpBase::BuildPost(std::ostream &stream, const std::string &data)
{
    stream << "POST " << req.path << " HTTP/1.1\r\n";
    stream << "Host: " << req.domain << "\r\n";
    stream << "Accept: */*\r\n";
    stream << "Content-Type:application/x-www-form-urlencoded\r\n";
    stream << "Content-Length: " << data.length() << "\r\n";
    stream << "Connection: close\r\n\r\n";
    stream << data;
}

template <typename _TcpTy> void HttpBase::WriteRequest(_TcpTy &socket)
{
    boost::asio::streambuf buffer;
    std::ostream ostr(&buffer);
    ostr << "GET " << req.path << " HTTP/1.1\r\n";
    ostr << "Host: " << req.domain << "\r\n";
    ostr << "Accept: */*\r\n";
    ostr << "Connection: close\r\n\r\n";
    boost::asio::write(socket, buffer);
}

template <typename _TcpTy> void HttpBase::ReadResponse(_TcpTy &socket)
{
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, "\r\n");

    std::istream istr(&buffer);
    res = new HttpResponse;
    res->headers["Content-Length"] = "0";
    istr >> res->httpVersion;
    std::cout << res->httpVersion << std::endl;

    istr >> res->status;
    std::cout << res->status << std::endl;

    std::getline(istr, res->statusMessage);
    std::cout << res->statusMessage << std::endl;

    if (!istr || res->httpVersion.substr(0, 5) != "HTTP/")
    {
        printf("响应无效\n");
    }

    if (res->status == 400)
    {
        return;
    }

    std::string header, key, value;

    boost::asio::read_until(socket, buffer, "\r\n\r\n");

    while (std::getline(istr, header) && header != "\r")
    {
        auto pos = header.find(": ");
        key = header.substr(0, pos);
        value = header.substr(pos + 2);
        res->headers[key] = value;
    }

    size_t ContentLength = std::atoi(res->headers["Content-Length"].c_str());
    auto size = buffer.size();

    if (size > 0)
    {
        std::cout << "left size is " << size << std::endl;
        // istr.unsetf(std::ios_base::skipws);
        header.resize(size);
        istr.read(header.data(), size);
        res->body.swap(header);
    }

    boost::system::error_code error;

    while (boost::asio::read(socket, buffer, boost::asio::transfer_at_least(1), error))
    {
        size = buffer.size();
        header.resize(size);
        istr.read(header.data(), size);
        res->body.append(header);
        if (ContentLength != 0)
        {
            std::cout << size << "  Byte  " << (size * 100) / ContentLength << "%" << std::endl;
        }
    }

    if (error != boost::asio::error::eof)
    {
        // std::cerr << "Exception: " << error.what() << std::endl;
        throw boost::system::system_error(error);
    }
}

HttpGet::HttpGet()
{
}

const HttpResponse *HttpGet::Get(const std::string &url)
{
    if (res)
    {
        delete res;
        res = nullptr;
    }

    ParseUrl(url);

    if (req.port == 443)
    {
        GetHttps();
    }
    else
    {
        GetHttp();
    }

    return res;
}

void HttpGet::GetHttp()
{
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(req.domain, std::to_string(req.port));
    auto iter = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, iter);

    WriteRequest(socket);
    ReadResponse(socket);
}

void HttpGet::GetHttps()
{
#ifdef __linux__
    try
    {
        boost::asio::io_service io_service;

        ssl::context ctx(ssl::context::sslv23);
        ctx.set_default_verify_paths();
        load_system_certs_on_windows(SSL_CTX_get_cert_store(ctx.native_handle()));

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(req.domain, std::to_string(req.port));
        auto iter = resolver.resolve(query);

        ssl::stream<tcp::socket> socket(io_service, ctx);
        boost::asio::connect(socket.lowest_layer(), iter);

        socket.set_verify_mode(ssl::verify_peer);
        socket.set_verify_callback(ssl::host_name_verification(req.domain));
        socket.lowest_layer().set_option(tcp::no_delay(true));
        socket.handshake(ssl::stream<tcp::socket>::client);

        WriteRequest(socket);
        ReadResponse(socket);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
#elif defined(_WIN32)
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    hSession = WinHttpOpen(TEXT("Microsoft Internet Explorer"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession)
    {
#ifdef UNICODE
        std::wstring url(req.domain.begin(), req.domain.end());
#else
        const std::string &url = req.domain;
#endif
        hConnect = WinHttpConnect(hSession, url.c_str(),
                                  INTERNET_DEFAULT_HTTP_PORT, /// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                  0);
    }
    else
    {
        printf("Invalid WinHTTP-session handle\n");
    }
    if (hConnect)
    {
        std::wstring path(req.path.begin(), req.path.end());
        hRequest = WinHttpOpenRequest(hConnect, TEXT("GET"), path.c_str(), NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }
    else
    {
        printf("Invalid connection handle\n");
    }

    // DWORD dwTimeOut = timeOut;
    DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
                    SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

    // BOOL bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(DWORD));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0);

    // bRet = WinHttpAddRequestHeader(hRequest, strHeader.c_str(), strHeader.length(),
    // WINHTTP_ADDREQ_FLAG_ADD|WINHTTP_ADDREQ_FLAG_REPLACE);

    BOOL bRet = FALSE;

    if (hRequest)
    {
        bRet = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }
    else
    {
        std::cout << "Invalid request handle\n";
    }
    if (bRet)
    {
        bRet = WinHttpReceiveResponse(hRequest, NULL);
    }
    else
    {
        std::cout << "Request failed\n";
    }
    if (bRet)
    {
        // DWORD dwSize;
        constexpr int bufferSize = 1024;
        res = new HttpResponse;
        res->status = 200;
        while (true)
        {
            //            dwSize = 0;
            char pszOutBuffer[bufferSize];
            DWORD dwDownloaded = 0;
            //            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            //            {
            //                std::cout << "Error " << GetLastError() << " in WinHttpQueryDataAvailable.\n";
            //                break;
            //            }
            // No more available data.
            //            if (!dwSize)
            //                break;
            // Read the Data.
            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, bufferSize, &dwDownloaded))
            {
                res->status = 404;
                std::cout << "Error " << GetLastError() << " in WinHttpReadData.\n";
            }
            else if (dwDownloaded)
            {
                res->body.insert(res->body.end(), pszOutBuffer, pszOutBuffer + dwDownloaded);
                // std::cout <<  pszOutBuffer << std::endl;
            }
            // This condition should never be reached since WinHttpQueryDataAvailable
            // reported that there are bits to read.
            if (!dwDownloaded)
                break;
        }
    }
    else
    {
        // Report any errors.
        std::cout << "Error " << GetLastError() << " has occurred.\n";
    }
    if (hRequest)
        WinHttpCloseHandle(hRequest);
    if (hConnect)
        WinHttpCloseHandle(hConnect);
    if (hSession)
        WinHttpCloseHandle(hSession);
#endif
}

HttpPost::HttpPost(const std::string &body)
{
    req.body = body;
}

const HttpResponse *HttpPost::Post(const std::string &url)
{
    using boost::asio::ip::tcp;
    if (res)
    {
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

    ReadResponse(socket);

    return res;
}

} // namespace httplib
