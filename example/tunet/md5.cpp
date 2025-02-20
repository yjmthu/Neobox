#include "md5.hpp"
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>


uint32_t t(uint32_t n, uint32_t t) {
  uint32_t r = (0xFFFF & n) + (0xFFFF & t);
  return ((n >> 16) + (t >> 16) + (r >> 16)) << 16 | (0xFFFF & r);
}

uint32_t r(uint32_t n, uint32_t t) { return (n << t) | (n >> (32 - t)); }

uint32_t e(uint32_t n, uint32_t e, uint32_t o, uint32_t u, uint32_t c,
           uint32_t f) {
  return t(r(t(t(e, n), t(u, f)), c), o);
}

uint32_t o(uint32_t n, uint32_t t, uint32_t r, uint32_t o, uint32_t u,
           uint32_t c, uint32_t f) {
  return e((t & r) | (~t & o), n, t, u, c, f);
}

uint32_t u(uint32_t n, uint32_t t, uint32_t r, uint32_t o, uint32_t u,
           uint32_t c, uint32_t f) {
  return e((t & o) | (r & ~o), n, t, u, c, f);
}

uint32_t c(uint32_t n, uint32_t t, uint32_t r, uint32_t o, uint32_t u,
           uint32_t c, uint32_t f) {
  return e(t ^ r ^ o, n, t, u, c, f);
}

uint32_t f(uint32_t n, uint32_t t, uint32_t r, uint32_t o, uint32_t u,
           uint32_t c, uint32_t f) {
  return e(r ^ (t | ~o), n, t, u, c, f);
}

using namespace std;

vector<uint32_t> i(vector<uint32_t>n, uint32_t r) {
  n[r >> 5] |= 128 << r % 32,
  n[14 + ((r + 64) >> 9 << 4)] = r;
  uint32_t e, i, a, d, h, l = 1732584193, g = -271733879, v = -1732584194, m = 271733878;
  for (e = 0; e < n.size(); e += 16) {
    i = l,
    a = g,
    d = v,
    h = m,
    g = f(g = f(g = f(g = f(g = c(g = c(g = c(g = c(g = u(g = u(g = u(g = u(g = o(g = o(g = o(g = o(g, v = o(v, m = o(m, l = o(l, g, v, m, n[e], 7, -680876936), g, v, n[e + 1], 12, -389564586), l, g, n[e + 2], 17, 606105819), m, l, n[e + 3], 22, -1044525330), v = o(v, m = o(m, l = o(l, g, v, m, n[e + 4], 7, -176418897), g, v, n[e + 5], 12, 1200080426), l, g, n[e + 6], 17, -1473231341), m, l, n[e + 7], 22, -45705983), v = o(v, m = o(m, l = o(l, g, v, m, n[e + 8], 7, 1770035416), g, v, n[e + 9], 12, -1958414417), l, g, n[e + 10], 17, -42063), m, l, n[e + 11], 22, -1990404162), v = o(v, m = o(m, l = o(l, g, v, m, n[e + 12], 7, 1804603682), g, v, n[e + 13], 12, -40341101), l, g, n[e + 14], 17, -1502002290), m, l, n[e + 15], 22, 1236535329), v = u(v, m = u(m, l = u(l, g, v, m, n[e + 1], 5, -165796510), g, v, n[e + 6], 9, -1069501632), l, g, n[e + 11], 14, 643717713), m, l, n[e], 20, -373897302), v = u(v, m = u(m, l = u(l, g, v, m, n[e + 5], 5, -701558691), g, v, n[e + 10], 9, 38016083), l, g, n[e + 15], 14, -660478335), m, l, n[e + 4], 20, -405537848), v = u(v, m = u(m, l = u(l, g, v, m, n[e + 9], 5, 568446438), g, v, n[e + 14], 9, -1019803690), l, g, n[e + 3], 14, -187363961), m, l, n[e + 8], 20, 1163531501), v = u(v, m = u(m, l = u(l, g, v, m, n[e + 13], 5, -1444681467), g, v, n[e + 2], 9, -51403784), l, g, n[e + 7], 14, 1735328473), m, l, n[e + 12], 20, -1926607734), v = c(v, m = c(m, l = c(l, g, v, m, n[e + 5], 4, -378558), g, v, n[e + 8], 11, -2022574463), l, g, n[e + 11], 16, 1839030562), m, l, n[e + 14], 23, -35309556), v = c(v, m = c(m, l = c(l, g, v, m, n[e + 1], 4, -1530992060), g, v, n[e + 4], 11, 1272893353), l, g, n[e + 7], 16, -155497632), m, l, n[e + 10], 23, -1094730640), v = c(v, m = c(m, l = c(l, g, v, m, n[e + 13], 4, 681279174), g, v, n[e], 11, -358537222), l, g, n[e + 3], 16, -722521979), m, l, n[e + 6], 23, 76029189), v = c(v, m = c(m, l = c(l, g, v, m, n[e + 9], 4, -640364487), g, v, n[e + 12], 11, -421815835), l, g, n[e + 15], 16, 530742520), m, l, n[e + 2], 23, -995338651), v = f(v, m = f(m, l = f(l, g, v, m, n[e], 6, -198630844), g, v, n[e + 7], 10, 1126891415), l, g, n[e + 14], 15, -1416354905), m, l, n[e + 5], 21, -57434055), v = f(v, m = f(m, l = f(l, g, v, m, n[e + 12], 6, 1700485571), g, v, n[e + 3], 10, -1894986606), l, g, n[e + 10], 15, -1051523), m, l, n[e + 1], 21, -2054922799), v = f(v, m = f(m, l = f(l, g, v, m, n[e + 8], 6, 1873313359), g, v, n[e + 15], 10, -30611744), l, g, n[e + 6], 15, -1560198380), m, l, n[e + 13], 21, 1309151649), v = f(v, m = f(m, l = f(l, g, v, m, n[e + 4], 6, -145523070), g, v, n[e + 11], 10, -1120210379), l, g, n[e + 2], 15, 718787259), m, l, n[e + 9], 21, -343485551),
    l = t(l, i),
    g = t(g, a),
    v = t(v, d),
    m = t(m, h);
  }
  return vector<uint32_t>{ l, g, v, m };
}

string a(const vector<uint32_t> &n) {
  string r = "";
  size_t e = 32 * n.size();
  for (size_t t = 0; t < e; t += 8) {
    r += static_cast<char>((n[t >> 5] >> (t % 32)) & 255);
  }
  return r;
}

vector<uint32_t> d(const string &n) {
  vector<uint32_t> r((n.length() >> 2)); // Equivalent to (n.length() / 4)
  for (size_t t = 0; t < r.size(); ++t) {
    r[t] = 0;
  }

  size_t e = 8 * n.length();
  for (size_t t = 0; t < e; t += 8) {
    r[t >> 5] |= (static_cast<uint32_t>(n[t / 8]) & 255) << (t % 32);
  }
  return r;
}

string h(const string &n) { return a(i(d(n), 8 * n.length())); }

string l(const string &n, const string &t) {
  vector<uint32_t> o = d(n);
  vector<uint32_t> u(16);
  vector<uint32_t> c(16);

  if (o.size() > 16) {
    o = i(o, 8 * n.length());
  }

  for (size_t r = 0; r < 16; ++r) {
    u[r] = 909522486 ^ o[r];
    c[r] = 1549556828 ^ o[r];
  }

  vector<uint32_t> e = i(u, 512 + 8 * t.length());
  e.insert(e.end(), d(t).begin(), d(t).end()); // Concatenate u and d(t)

  vector<uint32_t> combined;
  combined.insert(combined.end(), c.begin(), c.end());
  combined.insert(combined.end(), e.begin(), e.end());

  return a(i(combined, 640));
}

string g(const string &n) {
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

string p(const string &n) { return g(m(n)); }

string s(const string &n, const string &t) { return l(v(n), v(t)); }

string C(const string &n, const string &t) { return g(s(n, t)); }

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
