#-------------------------------------------------
#
# Project created by QtCreator 2017-02-10T21:21:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RGMResearcher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
		mainwindow.cpp \
	clusterisator.cpp \
    graphutility.cpp \
    graphview.cpp

HEADERS  += mainwindow.h \
	clusterisator.h \
	graph.h \
	graphutility.h \
	graphview.h

FORMS    += mainwindow.ui \
    graphview.ui

INCLUDEPATH += "C:/Graphviz2.38/include/graphviz" \
	"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v8.0/include"

LIBS += "-LC:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v8.0/lib/Win32"

release {
    LIBS += "-LC:/Graphviz2.38/lib/release/lib"
}

debug {
    LIBS += "-LC:/Graphviz2.38/lib/debug/lib"
}
