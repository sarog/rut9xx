set(warehouse_SRC
    warehouse.c
)
add_library ( warehouseext SHARED ${warehouse_SRC})
if(BUILD_AND_INSTALL_STATIC)
add_library ( warehouseext_static STATIC ${warehouse_SRC})
target_link_libraries (warehouseext_static LINK_PUBLIC twCSdk_static)
endif()
target_link_libraries (warehouseext LINK_PUBLIC twCSdk)

if(BUILD_AND_INSTALL_STATIC)
target_include_directories (warehouseext_static PUBLIC .)
endif()
target_include_directories (warehouseext PUBLIC .)

if (${OS} MATCHES "windows")
    ADD_CUSTOM_COMMAND(TARGET warehouseext
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ../ExtUseExample/ext
            COMMAND ${CMAKE_COMMAND} -E copy $(TargetPath) ../ExtUseExample/ext/.
            )
#    install(TARGETS warehouseext DESTINATION ../ExtUseExample/ext COMPONENT libraries)
else()
    ADD_CUSTOM_COMMAND(TARGET warehouseext
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ../ExtUseExample/ext
            COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_CURRENT_BINARY_DIR}/lib*.* ../ExtUseExample/ext/.
            )
endif()
