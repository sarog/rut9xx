file(GLOB_RECURSE unity_SRC "*.c")
add_library ( unity ${unity_SRC})
target_include_directories (unity PUBLIC .)
