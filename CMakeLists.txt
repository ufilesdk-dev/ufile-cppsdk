cmake_minimum_required(VERSION 3.10)

# set the project name
project(ufile-cppsdk)

include_directories(${PROJECT_SOURCE_DIR}/ucloud)
include_directories(${PROJECT_SOURCE_DIR}/ucloud/include)

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
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/api.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/apibase.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/common.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/delete.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/mput.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/put.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/download.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/base64.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/config.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/digest.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/errno.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/hmac_sha1.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/http.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/mimetype.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/json_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/network_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/stream_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/string_util.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/urlcodec.h
    ${PROJECT_SOURCE_DIR}/ucloud/include/ufile-cppsdk/list.h
    )

install(FILES ${head_files} DESTINATION include/ufile-cppsdk/)
