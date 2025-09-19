##
# @file CMakeLists.txt
# @brief 
#/

# APP_PATH
set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})

# APP_NAME
get_filename_component(APP_NAME ${APP_PATH} NAME)

# APP_SRCS
aux_source_directory(${APP_PATH}/src APP_SRCS)

########################################
# Target Configure
########################################
add_library(${EXAMPLE_LIB})

target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_SRCS}
    )

target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_INC}
        ${APP_PATH}/include
        ${TOP_SOURCE_DIR}/src/libaliyun_sdk/include
    )

# Link with the libaliyun_sdk module (which includes sdk_test functionality)
target_link_libraries(${EXAMPLE_LIB} 
    libaliyun_sdk
)

# Make sure the prebuilt lib is available in the platform lib search dir and included by name
file(COPY ${TOP_SOURCE_DIR}/src/libaliyun_sdk/prebuilt_libs/libsdk_test.a
     DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
file(COPY ${TOP_SOURCE_DIR}/src/libaliyun_sdk/prebuilt_libs/libaliyun_sdk.a
    DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
file(COPY ${TOP_SOURCE_DIR}/src/libaliyun_sdk/prebuilt_libs/libtinycrypt.a
    DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})


set(PLATFORM_NEED_LIBS "${PLATFORM_NEED_LIBS} sdk_test")
set(PLATFORM_NEED_LIBS "${PLATFORM_NEED_LIBS} aliyun_sdk")
set(PLATFORM_NEED_LIBS "${PLATFORM_NEED_LIBS} tinycrypt")
