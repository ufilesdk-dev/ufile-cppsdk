#ifndef _H_UFILESDK_CPP_UCLOUD_API_PUT_
#define _H_UFILESDK_CPP_UCLOUD_API_PUT_

#include "http/http.h"

namespace ucloud {
namespace cppsdk {
namespace api    {

class APIBase {

public:
    APIBase();
    ~APIBase();
protected:
    ucloud::cppsdk::http::HTTPRequest *m_http;
};

}
}
}

#endif
