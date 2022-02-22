# Generated from examples.pro.

qt_examples_build_begin()

if(TARGET Qt::Scxml)
    add_subdirectory(scxml)
endif()
if(TARGET Qt::StateMachine)
    add_subdirectory(statemachine)
endif()

qt_examples_build_end()
