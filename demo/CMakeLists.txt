set(demo_source_files
    main.cpp
    )

set(sdk_lib ufilecppsdk)

add_executable(demo ${demo_source_files})

target_link_libraries(demo
	              curl
		      magic
		      json-c
                      ${sdk_lib})
