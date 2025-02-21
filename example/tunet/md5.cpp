#include "md5.hpp"
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

constexpr uint32_t s11 = 7, s12 = 12, s13 = 17, s14 = 22;
constexpr uint32_t s21 = 5, s22 = 9, s23 = 14, s24 = 20;
constexpr uint32_t s31 = 4, s32 = 11, s33 = 16, s34 = 23;
constexpr uint32_t s41 = 6, s42 = 10, s43 = 15, s44 = 21;


constexpr inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & y) | (~x & z));
}

constexpr inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & z) | (y & ~z));
}

constexpr inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) {
  return (x ^ y ^ z);
}

constexpr inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) {
  return (y ^ (x | ~z));
}

inline void ROTATELEFT(uint32_t& a, uint32_t t) { a = (a << t) | (a >> (32 - t)); }

inline void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
  a += F(b, c, d) + x + ac;
  ROTATELEFT(a, s);
  a += b;
}

inline void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
  a += G(b, c, d) + x + ac;
  ROTATELEFT(a, s);
  a += b;
}

inline void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
  a += H(b, c, d) + x + ac;
  ROTATELEFT(a, s);
  a += b;
}

inline void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
  a += I(b, c, d) + x + ac;
  ROTATELEFT(a, s);
  a += b;
}

using namespace std;

vector<uint32_t> transform(vector<uint32_t>n, uint32_t r) {
  n.resize((r >> 5) + 1, 0);
  n.back() |= 128 << r % 32,
  n[14 + ((r + 64) >> 9 << 4)] = r;
  uint32_t e, a, b, c, d, A = 0x67452301, B = 0xEFCDAB89, C = 0x98BADCFE, D = 0x10325476;
  for (e = 0; e < n.size(); e += 16) {
    a = A;
    b = B;
    c = C;
    d = D;

    FF(a, b, c, d, n[e + 0], s11, 0xD76AA478);
    FF(d, a, b, c, n[e + 1], s12, 0xE8C7B756);
    FF(c, d, a, b, n[e + 2], s13, 0x242070DB);
    FF(b, c, d, a, n[e + 3], s14, 0xC1BDCEEE);
    FF(a, b, c, d, n[e + 4], s11, 0xF57C0FAF);
    FF(d, a, b, c, n[e + 5], s12, 0x4787C62A);
    FF(c, d, a, b, n[e + 6], s13, 0xA8304613);
    FF(b, c, d, a, n[e + 7], s14, 0xFD469501);
    FF(a, b, c, d, n[e + 8], s11, 0x698098D8);
    FF(d, a, b, c, n[e + 9], s12, 0x8B44F7AF);
    FF(c, d, a, b, n[e + 10], s13, 0xFFFF5BB1);
    FF(b, c, d, a, n[e + 11], s14, 0x895CD7BE);
    FF(a, b, c, d, n[e + 12], s11, 0x6B901122);
    FF(d, a, b, c, n[e + 13], s12, 0xFD987193);
    FF(c, d, a, b, n[e + 14], s13, 0xA679438E);
    FF(b, c, d, a, n[e + 15], s14, 0x49B40821);

    GG(a, b, c, d, n[e + 1], s21, 0xF61E2562);
    GG(d, a, b, c, n[e + 6], s22, 0xC040B340);
    GG(c, d, a, b, n[e + 11], s23, 0x265E5A51);
    GG(b, c, d, a, n[e + 0], s24, 0xE9B6C7AA);
    GG(a, b, c, d, n[e + 5], s21, 0xD62F105D);
    GG(d, a, b, c, n[e + 10], s22, 0x2441453);
    GG(c, d, a, b, n[e + 15], s23, 0xD8A1E681);
    GG(b, c, d, a, n[e + 4], s24, 0xE7D3FBC8);
    GG(a, b, c, d, n[e + 9], s21, 0x21E1CDE6);
    GG(d, a, b, c, n[e + 14], s22, 0xC33707D6);
    GG(c, d, a, b, n[e + 3], s23, 0xF4D50D87);
    GG(b, c, d, a, n[e + 8], s24, 0x455A14ED);
    GG(a, b, c, d, n[e + 13], s21, 0xA9E3E905);
    GG(d, a, b, c, n[e + 2], s22, 0xFCEFA3F8);
    GG(c, d, a, b, n[e + 7], s23, 0x676F02D9);
    GG(b, c, d, a, n[e + 12], s24, 0x8D2A4C8A);

    HH(a, b, c, d, n[e + 5], s31, 0xFFFA3942);
    HH(d, a, b, c, n[e + 8], s32, 0x8771F681);
    HH(c, d, a, b, n[e + 11], s33, 0x6D9D6122);
    HH(b, c, d, a, n[e + 14], s34, 0xFDE5380C);
    HH(a, b, c, d, n[e + 1], s31, 0xA4BEEA44);
    HH(d, a, b, c, n[e + 4], s32, 0x4BDECFA9);
    HH(c, d, a, b, n[e + 7], s33, 0xF6BB4B60);
    HH(b, c, d, a, n[e + 10], s34, 0xBEBFBC70);
    HH(a, b, c, d, n[e + 13], s31, 0x289B7EC6);
    HH(d, a, b, c, n[e + 0], s32, 0xEAA127FA);
    HH(c, d, a, b, n[e + 3], s33, 0xD4EF3085);
    HH(b, c, d, a, n[e + 6], s34, 0x4881D05);
    HH(a, b, c, d, n[e + 9], s31, 0xD9D4D039);
    HH(d, a, b, c, n[e + 12], s32, 0xE6DB99E5);
    HH(c, d, a, b, n[e + 15], s33, 0x1FA27CF8);
    HH(b, c, d, a, n[e + 2], s34, 0xC4AC5665);

    II(a, b, c, d, n[e + 0], s41, 0xF4292244);
    II(d, a, b, c, n[e + 7], s42, 0x432AFF97);
    II(c, d, a, b, n[e + 14], s43, 0xAB9423A7);
    II(b, c, d, a, n[e + 5], s44, 0xFC93A039);
    II(a, b, c, d, n[e + 12], s41, 0x655B59C3);
    II(d, a, b, c, n[e + 3], s42, 0x8F0CCC92);
    II(c, d, a, b, n[e + 10], s43, 0xFFEFF47D);
    II(b, c, d, a, n[e + 1], s44, 0x85845DD1);
    II(a, b, c, d, n[e + 8], s41, 0x6FA87E4F);
    II(d, a, b, c, n[e + 15], s42, 0xFE2CE6E0);
    II(c, d, a, b, n[e + 6], s43, 0xA3014314);
    II(b, c, d, a, n[e + 13], s44, 0x4E0811A1);
    II(a, b, c, d, n[e + 4], s41, 0xF7537E82);
    II(d, a, b, c, n[e + 11], s42, 0xBD3AF235);
    II(c, d, a, b, n[e + 2], s43, 0x2AD7D2BB);
    II(b, c, d, a, n[e + 9], s44, 0xEB86D391);

    A += a;
    B += b;
    C += c;
    D += d;
  }
  
  return {A, B, C, D};
}

string decode(const vector<uint32_t> &n) {
  string r = "";
  size_t e = 32 * n.size();
  for (size_t t = 0; t < e; t += 8) {
    r += static_cast<char>((n[t >> 5] >> (t % 32)) & 255);
  }
  return r;
}

vector<uint32_t> encode(const string &n) {
  vector<uint32_t> r((n.length() >> 2), 0); // Equivalent to (n.length() / 4)

  size_t e = 8 * n.length(); // Total number of bits
  for (size_t t = 0; t < e; t += 8) {
    // t >> 5 is equivalent to t / 32, and t % 32 is the remainder of t / 32
    r[t >> 5] |= (static_cast<uint32_t>(n[t / 8]) & 255) << (t % 32);
  }
  return r;
}

string h(const string &n) { return decode(transform(encode(n), 8 * n.length())); }

string l(const string &a, const string &b) {
  vector<uint32_t> o = encode(a);
  vector<uint32_t> u(16);
  vector<uint32_t> c(16);

  if (o.size() > 16) {
    o = transform(o, 8 * a.length());
  }
  o.resize(16, 0);

  for (size_t r = 0; r < 16; ++r) {
    u[r] = 0x36363636 ^ o[r];
    c[r] = 0x5c5c5c5c ^ o[r];
  }

  vector<uint32_t> s(u.begin(), u.end()), t = encode(b);
  s.insert(s.end(), t.begin(), t.end());
  vector<uint32_t> e = transform(s, 512 + 8 * b.length()); // 512 = 32 * 16

  vector<uint32_t> combined;
  combined.reserve(32);
  combined.insert(combined.end(), c.begin(), c.end()); // 32 * 16
  combined.insert(combined.end(), e.begin(), e.end()); // 32 * 4

  return decode(transform(combined, 640));  // 640 = 32 * (16 + 4)
}

string format(const string &n) {
  string e = "";
  for (size_t r = 0; r < n.length(); ++r) {
    unsigned char t = n[r];
    e += "0123456789abcdef"[(t >> 4) & 15];
    e += "0123456789abcdef"[t & 15];
  }
  return e;
}

// Forward declarations for functions that will be defined later
string v(const string &n);
string m(const string &n);
string p(const string &n);
string s(const string &n, const string &t);
string C(const string &n, const string &t);
string md5(const string &n, const string &t = "",
                bool r = false);

// Helper function for UTF-8 encoding (simulating encodeURIComponent)
string encodeURIComponent(const string &str) {
  ostringstream encoded;
  encoded.fill('0');
  encoded << hex;

  for (unsigned char c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
    } else {
      encoded << uppercase;
      encoded << '%' << setw(2) << int(c);
      encoded << nouppercase;
    }
  }

  return encoded.str();
}

// Helper function for UTF-8 decoding (simulating unescape + decodeURIComponent)
string unescape(const string &str) {
  string result;
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '%') {
      if (i + 2 < str.length()) {
        string hex_str = str.substr(i + 1, 2);
        try {
          int hex_val = stoi(hex_str, nullptr, 16);
          result += static_cast<char>(hex_val);
          i += 2;
        } catch (const invalid_argument &e) {
          // If not a valid hex code, just append the '%' and continue
          result += str[i];
        } catch (const out_of_range &e) {
          // Handle out-of-range errors
          result += str[i];
        }
      } else {
        result += str[i];
      }
    } else {
      result += str[i];
    }
  }
  return result;
}

// Implementation of the converted functions
string v(const string &n) { return unescape(encodeURIComponent(n)); }

string m(const string &n) { return h(v(n)); }

string p(const string &n) { return format(m(n)); }

string s(const string &n, const string &t) { return l(v(n), v(t)); }

string C(const string &n, const string &t) { return format(s(n, t)); }

string md5(const string &n, const string &t, bool r) {
  if (!t.empty()) {
    if (r) {
      return s(t, n);
    } else {
      return C(t, n);
    }
  } else {
    if (r) {
      return m(n);
    } else {
      return p(n);
    }
  }
}

int test() {
  string input_string = "hello world";
  string key_string = "key";

  std::cout << "========================\n";
#if 1
  cout << "md5(\"" << input_string << "\") = " << md5(input_string) << endl;
  cout << "md5(\"" << input_string << "\", \"" << key_string
       << "\") = " << md5(input_string, key_string) << endl;
  cout << "md5(\"" << input_string << "\", \"" << key_string
       << "\", true) = " << md5(input_string, key_string, true) << endl;
  cout << "md5(\"" << input_string
       << "\", \"\", true) = " << md5(input_string, "", true) << endl;
#endif

  return 0;
}
