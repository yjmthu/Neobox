#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <filesystem>
#include <xstring>

namespace HttpLib {
bool IsOnline();

long Get(const char* url, std::u8string& data);
inline long Get(const std::u8string& url, std::u8string& data) {
  return Get(reinterpret_cast<const char*>(url.c_str()), data);
}
inline long Get(const std::string& url, std::u8string& data) {
  return Get(url.c_str(), data);
}
long Get(const char* url, std::filesystem::path data);
inline long Get(const std::u8string& url, std::filesystem::path data) {
  return Get(reinterpret_cast<const char*>(url.c_str()), data);
}
inline long Get(const std::string& url, std::filesystem::path data) {
  return Get(url.c_str(), data);
}
long Gets(const char* url, std::filesystem::path data);
inline long Gets(const std::u8string& url, std::filesystem::path data) {
  return Gets(reinterpret_cast<const char*>(url.c_str()), data);
}
inline long Gets(const std::string& url, std::filesystem::path data) {
  return Gets(url.c_str(), data);
}
}  // namespace HttpLib

#endif
