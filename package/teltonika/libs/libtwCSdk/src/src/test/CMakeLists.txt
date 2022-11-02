# Remove strings matching given regular expression from a list.
# @param(in,out) aItems Reference of a list variable to filter.
# @param aRegEx Value of regular expression to match.
function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)

file(GLOB_RECURSE unit_SRC "unit/*.c")
file(GLOB_RECURSE integration_SRC "integration/*.c")
file(GLOB_RECURSE integration_slow_SRC "integration_slow/*.c")
file(GLOB_RECURSE performance_SRC "performance/*.c")
file(GLOB_RECURSE shapes_unit_SRC "shapes/*.c")

if ( "${USE_GRAFANA}" STREQUAL "ON" )
    set(grafana_SRC graphite-c-client/graphite-client.c)
    add_definitions(-DUSE_GRAFANA)
endif()
if ( "${USE_GRAFANA}" STREQUAL "OFF" )
    set(grafana_SRC "")
endif()

if ( "${USE_HTML_GRAPHING}" STREQUAL "ON" )
    set(html_graphing_SRC chart-js-client/chart-js-client.c)
    add_definitions(-DUSE_HTML_GRAPHING)
endif()
if ( "${USE_HTML_GRAPHING}" STREQUAL "OFF" )
    set(html_graphing_SRC "")
endif()

set(test_SRC
        unity/unity.c
        unity/unity_fixture.c
        ${unity_SRC}
        ${unit_SRC}
        ${shapes_unit_SRC}
        ${integration_SRC}
        ${integration_slow_SRC}
        ${performance_SRC}
        ${src_SRC}
        ${grafana_SRC}
        ${html_graphing_SRC}
        src/testmain.c
        src/TestUtilities.c
        src/TestServices.c
        src/crossPlatformTestSupport.c
        ../examples/SteamSensor/src/SteamThing.c
        ../examples/Regression/src/RegressionThing.c
)

set(test_SRC_static
        unity/unity.c
        unity/unity_fixture.c
        ${unity_SRC}
        ${unit_SRC}
        ${shapes_unit_SRC}
        ${integration_SRC}
        ${integration_slow_SRC}
        ${performance_SRC}
        ${src_SRC}
        ${grafana_SRC}
        ${html_graphing_SRC}
        src/testmain.c
        src/TestUtilities.c
        src/TestServices.c
        src/crossPlatformTestSupport.c
        ../examples/SteamSensor/src/SteamThing.c
        ../examples/Regression/src/RegressionThing.c
)

if(RELEASE)
    filter_items(unit_SRC ".*Regression.*")
    filter_items(integration_SRC ".*Regression.*")
    filter_items(integration_slow_SRC ".*Regression.*")
    filter_items(performance_SRC ".*Regression.*")
    filter_items(test_SRC ".*Regression.*")
    filter_items(test_SRC_static ".*Regression.*")
    add_definitions(-DTW_RELEASE_BUILD)
endif()

include_directories(
    include
    macros
    unity
    graphite-c-client
    chart-js-client
    ${OPENSSL_INCLUDE_DIR}
    ../examples/warehouseextlib/
    ../examples/simpleextlib/
    ../examples/SteamSensor/src
    ../examples/Regression/src
)
add_executable (csdk_tests ${test_SRC})
target_link_libraries (csdk_tests LINK_PUBLIC twCSdk )

if(BUILD_AND_INSTALL_STATIC)
add_executable (csdk_tests_static ${test_SRC_static} )
target_link_libraries (csdk_tests_static LINK_PUBLIC twCSdk_static )
endif()


# copy required DLLs to same directory as shape tests so windows can find them
if (${OS} MATCHES "windows")
    add_custom_command(TARGET csdk_tests POST_BUILD
            COMMAND cmake -E copy_if_different "${CMAKE_BINARY_DIR}/src/$<CONFIGURATION>/twCSdk.dll" "${CMAKE_BINARY_DIR}/test/$<CONFIGURATION>/twCSdk.dll"
            COMMENT "Copying sdk dll to tests directory.")
    # Copy over openssl dll files
    add_custom_command(TARGET csdk_tests POST_BUILD
            COMMAND cmake -E copy_if_different "${OPENSSL_SSLEAY_BIN_PATH}" "${CMAKE_BINARY_DIR}/test/$<CONFIGURATION>/ssleay32.dll"
            COMMENT "Copying ssleay dll to tests directory.")
    add_custom_command(TARGET csdk_tests POST_BUILD
            COMMAND cmake -E copy_if_different "${OPENSSL_LIBEAY_BIN_PATH}" "${CMAKE_BINARY_DIR}/test/$<CONFIGURATION>/libeay32.dll"
            COMMENT "Copying libeay dll to tests directory.")
endif()

if (${OS} MATCHES "linux")
    # Copy over openssl libraries.
    add_custom_command(TARGET csdk_tests POST_BUILD
        COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libcrypto.so.1.0.0" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libssl.so.1.0.0" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying openssl to test directory.")
endif ()

if (${OS} MATCHES "macos")
    # Copy over openssl libraries.
    add_custom_command(TARGET csdk_tests POST_BUILD
        COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libcrypto.1.0.0.dylib" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND cmake -E copy_if_different "${OPENSSL_LIB_DIR}/libssl.1.0.0.dylib" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying openssl to test directory.")
endif()
