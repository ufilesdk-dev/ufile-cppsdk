// Package url parses URLs and implements query escaping.
// See RFC 3986.

#include <string>

namespace ucloud {
namespace cppsdk {
namespace utils {

bool ishex(char c) {

  return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
         ('A' <= c && c <= 'F');
}

int unhex(char c) {

  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  } else if ('A' <= c && c <= 'F') {
    return c - 'A' + 10;
  }
  return 0;
}

enum EncodingType {
  ENCODE_PATH = 1,
  ENCODE_USER_PASSWORD,
  ENCODE_QUERY_COMPONENT,
  ENCODE_FRAGMENT,
};

// Return true if the specified character should be escaped when
// appearing in a URL string, according to RFC 3986.
// When 'all' is true the full range of reserved characters are matched.
bool shouldEscape(char c, EncodingType mode) {

  // §2.3 Unreserved characters (alphanum)
  if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
      ('0' <= c && c <= '9')) {
    return false;
  }

  std::string s1 = "-_.~";
  std::string s2 = "$&+,/:;=?@";
  if (s1.find(c) != std::string::npos) { // §2.3 Unreserved characters (mark)
    return false;
  } else if (s2.find(c) !=
             std::string::npos) { // §2.2 Reserved characters (reserved)
    // Different sections of the URL allow a few of
    // the reserved characters to appear unescaped.
    switch (mode) {
    case ENCODE_PATH: // §3.3
      // The RFC allows : @ & = + $ but saves / ; , for assigning
      // meaning to individual path segments. This package
      // only manipulates the path as a whole, so we allow those
      // last two as well. That leaves only ? to escape.
      return c == '?';

    case ENCODE_USER_PASSWORD: // §3.2.2
      // The RFC allows ; : & = + $ , in userinfo, so we must escape only @ and
      // /. The parsing of userinfo treats : as special so we must escape that
      // too.
      return c == '@' || c == '/' || c == ':';

    case ENCODE_QUERY_COMPONENT: // §3.4
      // The RFC reserves (so we must escape) everything.
      return true;

    case ENCODE_FRAGMENT: // §4.1
      // The RFC text is silent but the grammar allows
      // everything, so escape nothing.
      return false;
    }
  }

  // Everything else must be escaped.
  return true;
}

// unescape unescapes a string; the mode specifies
// which section of the URL string is being unescaped.
int unescape(const EncodingType mode, const std::string &src,
             std::string *dst) {

  // Count %, check that they're well-formed.
  size_t n = 0;
  bool hasPlus = false;
  for (size_t i = 0; i < src.size();) {
    switch (src[i]) {
    case '%':
      n++;
      if (i + 2 >= src.size() || !ishex(src[i + 1]) || !ishex(src[i + 2])) {
        return -1;
      }
      i += 3;
      break;
    case '+':
      hasPlus = (mode == ENCODE_QUERY_COMPONENT);
      i++;
      break;
    default:
      i++;
      break;
    }
  }

  if (n == 0 && !hasPlus) {
    *dst = src;
    return 0;
  }

  for (size_t i = 0; i < src.size();) {
    switch (src[i]) {
    case '%':
      *dst += unhex(src[i + 1]) << 4 | unhex(src[i + 2]);
      i += 3;
      break;
    case '+':
      if (mode == ENCODE_QUERY_COMPONENT) {
        *dst += ' ';
      } else {
        *dst += '+';
      }
      i++;
      break;
    default:
      *dst += src[i];
      i++;
      break;
    }
  }
  return 0;
}

int escape(const EncodingType mode, const std::string &src, std::string *dst) {

  size_t spaceCount = 0, hexCount = 0;
  for (size_t i = 0; i < src.size(); i++) {
    char c = src[i];
    if (shouldEscape(c, mode)) {
      if (c == ' ' && mode == ENCODE_QUERY_COMPONENT) {
        spaceCount++;
      } else {
        hexCount++;
      }
    }
  }

  if (spaceCount == 0 && hexCount == 0) {
    *dst = src;
    return 0;
  }

  size_t j = 0;
  for (size_t i = 0; i < src.size(); i++) {
    char c = src[i];
    if (c == ' ' && mode == ENCODE_QUERY_COMPONENT) {
      *dst += "%20";
      j++;
    } else if (shouldEscape(c, mode)) {
      *dst += '%';
      *dst += "0123456789ABCDEF"[c >> 4];
      *dst += "0123456789ABCDEF"[c & 15];
      j += 3;
    } else {
      *dst += src[i];
      j++;
    }
  }
  return 0;
}

// QueryEscape escapes the string so it can be safely placed
// inside a URL query.
int QueryEscape(const std::string &src, std::string *dst) {

  std::string t;
  int ret = escape(ENCODE_QUERY_COMPONENT, src, &t);
  if (!ret)
    *dst = t;
  return ret;
}

// QueryUnescape does the inverse transformation of QueryEscape, converting
// %AB into the byte 0xAB and '+' into ' ' (space). It returns an error if
// any % is not followed by two hexadecimal digits.
int QueryUnescape(const std::string &src, std::string *dst) {

  std::string t;
  int ret = unescape(ENCODE_QUERY_COMPONENT, src, &t);
  if (!ret)
    *dst = t;
  return ret;
}

int PathEscape(const std::string &src, std::string *dst) {

  std::string t;
  int ret = escape(ENCODE_PATH, src, &t);
  if (!ret)
    *dst = t;
  return ret;
}

int PathUnescape(const std::string &src, std::string *dst) {

  std::string t;
  int ret = unescape(ENCODE_PATH, src, &t);
  if (!ret)
    *dst = t;
  return ret;
}

std::string EasyQueryEscape(const std::string &src) {

  std::string dst = src;
  QueryEscape(src, &dst);
  return dst;
}

std::string EasyQueryUnescape(const std::string &src) {

  std::string dst = src;
  QueryUnescape(src, &dst);
  return dst;
}

std::string EasyPathEscape(const std::string &src) {

  std::string dst = src;
  PathEscape(src, &dst);
  return dst;
}

std::string EasyPathUnescape(const std::string &src) {

  std::string dst = src;
  PathUnescape(src, &dst);
  return dst;
}

} // namespace utils
} // namespace cppsdk
} // namespace ucloud
