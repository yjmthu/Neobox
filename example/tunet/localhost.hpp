#ifndef LOCALHOST_H
#define LOCALHOST_H

#include <string>
#include <set>

typedef std::set<std::u8string> SuffixSet;
std::u8string GetLocalIPAddressWithDNSSuffix(SuffixSet dnsSuffix);

#endif // LOCALHOST_H
