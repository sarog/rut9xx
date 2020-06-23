set(simpleshape_SRC
    simpleshape.c
)
add_library ( simpleext SHARED ${simpleshape_SRC})
target_link_libraries (simpleext LINK_PUBLIC twCSdk)
target_include_directories (simpleext PUBLIC .)
# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
    if(COMPONENT)
        message(STATUS "Install component: \"${COMPONENT}\"")
        set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
    else()
        set(CMAKE_INSTALL_COMPONENT)
    endif()
endif()

if (${OS} MATCHES "windows")
    ADD_CUSTOM_COMMAND(TARGET simpleext
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ../ExtUseExample/ext
        COMMAND ${CMAKE_COMMAND} -E copy  $(TargetPath) ../ExtUseExample/ext/.
        )
else()
    ADD_CUSTOM_COMMAND(TARGET simpleext
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ../ExtUseExample/ext
            COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_CURRENT_BINARY_DIR}/lib*.* ../ExtUseExample/ext/.
            )
endif()