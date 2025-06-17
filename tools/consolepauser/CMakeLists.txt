add_executable(consolepauser)

if(WIN32)
    target_sources(consolepauser PRIVATE
        main.windows.cpp
    )

    target_link_libraries(consolepauser PRIVATE
        psapi
    )
endif()

if(UNIX)
    target_sources(consolepauser PRIVATE
        main.unix.cpp
    )
endif()

install(TARGETS consolepauser
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/${APP_NAME}
)
