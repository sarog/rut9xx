file(GLOB_RECURSE Unit_Tests_SRC "*.c")
add_library (Unit_Tests ${Unit_Tests_SRC})
add_library (Unit_TestsStatic ${Unit_Tests_SRC})

include_directories(../Test_Utils)

target_link_libraries (Unit_Tests LINK_PUBLIC twCSdk unity )
target_link_libraries (Unit_TestsStatic LINK_PUBLIC twCSdk_static unity )
#copy required DLLs to same directory as shape tests so windows can find them
if(WIN32)
  add_custom_command(TARGET Unit_Tests POST_BUILD
    COMMAND cmake -E copy_if_different "${CMAKE_BINARY_DIR}/src/$<CONFIGURATION>/twCSdk.dll" "${CMAKE_BINARY_DIR}/test/coreapi/$<CONFIGURATION>/twCSdk.dll"
     COMMENT "Copying sdk dll to tests directory.")
endif()
