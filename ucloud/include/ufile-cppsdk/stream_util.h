#ifndef _UFILESDK_CPP_UCLOUD_UTILS_STREAM_UTIL_
#define _UFILESDK_CPP_UCLOUD_UTILS_STREAM_UTIL_

#include <ostream>
#include <string>
#include <vector>
#include <streambuf>

namespace ucloud {
namespace cppsdk {
namespace utils {

enum {
  STREAM_TYPE_STD = 1,
  STREAM_TYPE_MULTI,
  STREAM_TYPE_LIMIT,
};

class UCloudOStream : public std::ostream {

public:
  UCloudOStream() {}
  explicit UCloudOStream(std::ostream *os)
      : m_type(STREAM_TYPE_STD), m_os(os) {}
  explicit UCloudOStream(int type) : m_type(type), m_os(NULL) {}
  UCloudOStream(std::ostream *os, int type) : m_type(type), m_os(os) {}
  ~UCloudOStream() {}

  int Type() { return m_type; }

  std::ostream *Stream() { return m_os; }

protected:
  int m_type;
  std::ostream *m_os;
};

class LimitWriteStream : public UCloudOStream {

public:
  LimitWriteStream(std::ostream *os, const size_t &limit)
      : UCloudOStream(os, STREAM_TYPE_LIMIT), m_reach_limit(false),
        m_limit(limit) {}
  ~LimitWriteStream() {}

  template <class T> LimitWriteStream &operator<<(const T &x) {

    if (m_reach_limit)
      return *this;

    if (m_os) {
      m_os->operator<<(x);
    }

    std::streampos pos = m_os->tellp();
    if (m_os->fail()) {
      m_reach_limit = true;
    } else if (pos >= std::streampos(m_limit)) {
      m_reach_limit = true;
      m_os->seekp(m_limit);
      if (!m_os->fail()) {
        m_os->operator<<(std::ends);
      }
    }
    return *this;
  }

  std::streambuf *rdbuf() { return m_os->rdbuf(); }

  std::streamsize sputn(const char *s, std::streamsize n) {
    return this->rdbuf()->sputn(s, n);
  }

private:
  bool m_reach_limit;
  size_t m_limit;
};

class MultiWriteStream : public UCloudOStream {

public:
  MultiWriteStream() : UCloudOStream(STREAM_TYPE_MULTI) { m_osv.clear(); }
  ~MultiWriteStream() {}

  MultiWriteStream &AddStream(UCloudOStream *os) {
    m_osv.push_back(os);
    return *this;
  }
  // this is not a virtual function,so it's dirty
  template <class T> MultiWriteStream &operator<<(const T &x) {

    for (size_t i = 0; i < m_osv.size(); ++i) {
      std::ostream *os = m_osv[i];
      if (os) {
        os->operator<<(x);
      }
    }
    return *this;
  }

  MultiWriteStream &operator<<(const std::string *s) {

    for (size_t i = 0; i < m_osv.size(); ++i) {
      UCloudOStream *os = m_osv[i];
      if (os) {
        if (os->Type() == STREAM_TYPE_LIMIT) {
          ((LimitWriteStream *)os)->sputn(s->c_str(), s->size());
        } else if (os->Type() == STREAM_TYPE_STD) {
          os->Stream()->rdbuf()->sputn(s->c_str(), s->size());
        }
      }
    }
    return *this;
  }

  std::streampos tellp() {
    if (!m_osv.empty()) {
      return m_osv[0]->Stream()->tellp();
    }
    return -1;
  }

private:
  std::vector<UCloudOStream *> m_osv;
};

class UCloudStreamBuf : public std::streambuf {
public:
  UCloudStreamBuf(const char *data, size_t size) {
    char *base = const_cast<char *>(data);
    // 设置左右边界以及读指针位置
    setg(base, base, base + size);
  }
};

} // namespace utils
} // namespace cppsdk
} // namespace ucloud

#endif
