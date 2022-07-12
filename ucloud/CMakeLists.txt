aux_source_directory(api api_srcs)
aux_source_directory(base64 base64_srcs)
aux_source_directory(config config_srcs)
aux_source_directory(digest digest_srcs)
aux_source_directory(errno errno_srcs)
aux_source_directory(hmac hmac_srcs)
aux_source_directory(http http_srcs)
aux_source_directory(utils utils_srcs)

set (sdk_srcs
     ${api_srcs}
     ${base64_srcs}
     ${config_srcs}
     ${digest_srcs}
     ${errno_srcs}
     ${hmac_srcs}
     ${http_srcs}
     ${utils_srcs}
     )

add_library(ufilecppsdk STATIC
            ${sdk_srcs}
            )

target_link_libraries(ufilecppsdk
	              tcmalloc
		      curl
		      magic)

install(TARGETS ufilecppsdk ARCHIVE DESTINATION lib)
