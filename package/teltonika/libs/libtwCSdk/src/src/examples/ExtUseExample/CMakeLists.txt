add_executable (ExtUseExample ExtUseExample.c)
target_link_libraries (ExtUseExample LINK_PUBLIC twCSdk)

if (${OS} MATCHES "windows")
    add_custom_command(TARGET ExtUseExample POST_BUILD
            COMMAND cmake -E copy_if_different "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/twCSdk.dll" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/twCSdk.dll"
            COMMENT "Copying C SDK dll to example directory.")
    # Copy over openssl dll files
    add_custom_command(TARGET ExtUseExample POST_BUILD
            COMMAND cmake -E copy_if_different "${OPENSSL_SSLEAY_BIN_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/ssleay32.dll"
            COMMENT "Copying ssleay dll to example directory.")
    add_custom_command(TARGET ExtUseExample POST_BUILD
            COMMAND cmake -E copy_if_different "${OPENSSL_LIBEAY_BIN_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/libeay32.dll"
            COMMENT "Copying ssleay dll to example directory.")
endif ()

if (${OS} MATCHES "linux")
    # Copy over openssl libraries.
    add_custom_command(TARGET ExtUseExample POST_BUILD
            COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libcrypto.so.1.0.0" "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libssl.so.1.0.0" "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Copying openssl to test directory.")
endif ()

if (${OS} MATCHES "macos")
    # Copy over openssl libraries.
    add_custom_command(TARGET ExtUseExample POST_BUILD
            COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libcrypto.1.0.0.dylib" "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libssl.1.0.0.dylib" "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Copying openssl to test directory.")
endif()
