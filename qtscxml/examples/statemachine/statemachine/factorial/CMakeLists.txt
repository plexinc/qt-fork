cmake_minimum_required(VERSION 3.16)
project(factorial LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/statemachine/factorial")

find_package(Qt6 REQUIRED COMPONENTS Core StateMachine)

qt_add_executable(factorial
    main.cpp
)

set_target_properties(factorial PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(factorial PUBLIC
    Qt::Core
    Qt::StateMachine
)

install(TARGETS factorial
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
