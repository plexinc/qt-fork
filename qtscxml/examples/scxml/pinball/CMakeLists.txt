cmake_minimum_required(VERSION 3.16)
project(pinball LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/pinball")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Scxml Widgets)

qt_add_executable(pinball
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

set_target_properties(pinball PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(pinball PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Scxml
    Qt::Widgets
)

# Statecharts:
qt6_add_statecharts(pinball
    pinball.scxml
)

install(TARGETS pinball
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
