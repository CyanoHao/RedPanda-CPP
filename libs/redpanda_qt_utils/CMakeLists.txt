qt_add_library(redpanda_qt_utils
    qt_utils/charsetinfo.cpp qt_utils/charsetinfo.h
    qt_utils/utils.cpp qt_utils/utils.h
)
target_link_libraries(redpanda_qt_utils PUBLIC
    Qt::Core
    Qt::Gui
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(redpanda_qt_utils PUBLIC
        Qt::Widgets
    )
endif()

if(WIN32)
    target_compile_definitions(redpanda_qt_utils PUBLIC
        _WIN32_WINNT=0x0501
    )
endif()

if(MSVC)
    target_compile_definitions(redpanda_qt_utils PUBLIC
        NOMINMAX
    )
endif()

install(TARGETS redpanda_qt_utils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
