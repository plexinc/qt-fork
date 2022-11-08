#####################################################################
## tst_cppgen Tool:
#####################################################################

qt_get_tool_target_name(target_name tst_cppgen)
qt_internal_add_tool(${target_name}
    SOURCES
        tst_cppgen.cpp
    PUBLIC_LIBRARIES
        Qt::Qml
        Qt::Scxml
        Qt::Test
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:cppgen.pro:<TRUE>:
# TEMPLATE = "app"
# _OPTION = "host_build"
