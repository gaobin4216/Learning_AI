cmake_minimum_required(VERSION 3.19)
project(Qt_20260618 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Qt_20260618
    WIN32 MACOSX_BUNDLE
    main.cpp
    MyWindow.cpp
    MyWindow.h
)

target_link_libraries(Qt_20260618
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS Qt_20260618
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Qt_20260618
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
