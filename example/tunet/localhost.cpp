#include <iostream>
#include "localhost.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "ws2_32.lib")

std::u8string GetLocalIPAddressWithDNSSuffix(SuffixSet dnsSuffix) {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    // Winsock 初始化失败
    return {};
  }

  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
    // 获取主机名失败
    WSACleanup();
    return {};
  }

  struct addrinfo hints, *result = NULL, *ptr = NULL;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // 使用IPv4或IPv6
  hints.ai_socktype = SOCK_STREAM;

  // 获取地址信息
  if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
    WSACleanup();
    return {};
  }

  std::u8string ipStr;
  char8_t ipBuffer[INET6_ADDRSTRLEN];
  for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    void *addr;
    // 检查地址族并获取正确的地址指针
    if (ptr->ai_family == AF_INET) { // IPv4
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;
      addr = &(ipv4->sin_addr);
#if 0
    } else if (ptr->ai_family == AF_INET6) { // IPv6
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ptr->ai_addr;
      addr = &(ipv6->sin6_addr);
#endif
    } else {
      continue;
    }

    // 将地址转换为字符串
    if (inet_ntop(ptr->ai_family, addr, reinterpret_cast<char *>(ipBuffer),
                  sizeof(ipBuffer)) == NULL) {
      continue;
    }

    // std::cout << (char*) ipBuffer << std::endl;

    // 检查DNS后缀是否匹配
    if (!dnsSuffix.empty()) {
      char8_t canonicalName[256];
      if (getnameinfo(ptr->ai_addr, ptr->ai_addrlen,
                      reinterpret_cast<char *>(canonicalName),
                      sizeof(canonicalName), NULL, 0, NI_NAMEREQD) == 0) {
        // std::cout << (char*) canonicalName << std::endl;
        std::u8string canonical(canonicalName);
        auto iter = std::find_if(dnsSuffix.cbegin(), dnsSuffix.cend(), [&canonical](auto suffix){
          return canonical.ends_with(suffix);
        });

        if (iter != dnsSuffix.end()) {
          ipStr = ipBuffer;
          break;
        }
      }
    } else {
      // 如果没有指定DNS后缀，则直接返回第一个IPv4地址
      ipStr = ipBuffer;
      break;
    }
  }

  freeaddrinfo(result); // 释放地址信息结构
  WSACleanup();         // 清理Winsock

  return ipStr;
}

static int test() {
  const SuffixSet dnsSuffix = { u8"tsinghua.edu.cn" }; // 替换为你的DNS后缀
  std::u8string ip = GetLocalIPAddressWithDNSSuffix(dnsSuffix);
  if (ip.empty()) {
    std::cout << "无法获取具有指定DNS后缀的本机IP地址" << std::endl;
  } else {
    std::cout << "具有指定DNS后缀的本机IP地址是: "
              << std::string(ip.begin(), ip.end()) << std::endl;
  }

  return 0;
}
