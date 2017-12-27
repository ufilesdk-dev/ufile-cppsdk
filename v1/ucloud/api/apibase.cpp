#include "apibase.h"

namespace ucloud {
namespace cppsdk {
namespace api    {

APIBase::APIBase() {

    m_http = new ucloud::cppsdk::http::HTTPRequest("GET", "/");
}

APIBase::~APIBase() {

    if (m_http) delete m_http;
}

}
}
}
