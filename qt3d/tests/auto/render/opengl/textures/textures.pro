TEMPLATE = app

TARGET = tst_textures

QT += core-private 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += tst_textures.cpp

include(../../../core/common/common.pri)
include(../../commons/commons.pri)

# Link Against OpenGL Renderer Plugin
include(../opengl_render_plugin.pri)
