cmake_minimum_required(VERSION 3.10)

# set the project name
project(ufile-cppsdk)

include_directories(${PROJECT_SOURCE_DIR}/ucloud)

link_directories("/usr/lib")
link_directories("/usr/local/lib")

# specify the C++ standard
set(CMAKE_CXX_STANATD 0x)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_DEMO "Build demo" ON)

add_subdirectory(ucloud)
if(BUILD_DEMO)
  add_subdirectory(demo)
endif()

set(head_files
    ${PROJECT_SOURCE_DIR}/ucloud/api/api.h
    ${PROJECT_SOURCE_DIR}/ucloud/api/apibase.h
    ${PROJECT_SOURCE_DIR}/ucloud/api/common.h
    ${PROJECT_SOURCE_DIR}/ucloud/api/delete.h
    ${PROJECT_SOURCE_DIR}/ucloud/api/mput.h
    ${PROJECT_SOURCE_DIR}/ucloud/api/put.h
    ${PROJECT_SOURCE_DIR}/ucloud/base64/base64.h
    ${PROJECT_SOURCE_DIR}/ucloud/config/config.h
    ${PROJECT_SOURCE_DIR}/ucloud/digest/digest.h
    ${PROJECT_SOURCE_DIR}/ucloud/errno/errno.h
    ${PROJECT_SOURCE_DIR}/ucloud/hmac/hmac_sha1.h
    ${PROJECT_SOURCE_DIR}/ucloud/http/http.h
    ${PROJECT_SOURCE_DIR}/ucloud/http/mimetype.h
    ${PROJECT_SOURCE_DIR}/ucloud/utils/json_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/utils/network_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/utils/stream_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/utils/string_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/utils/urlcodec.h
    )

install(FILES ${head_files} DESTINATION include/ufile-cppsdk/)
