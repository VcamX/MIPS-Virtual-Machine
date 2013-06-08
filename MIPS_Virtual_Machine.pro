OTHER_FILES +=

HEADERS += \
    mainwindow.h \
    data_type.h \
    loadingdialog.h \
    assembler.h \
    deassembler.h \
    qtcpu.h \
    keyboardtextedit.h \
    qtcpu_thread.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    loadingdialog.cpp \
    deassembler.cpp \
    assembler.cpp \
    qtcpu.cpp \
    keyboardtextedit.cpp \
    qtcpu_thread.cpp

FORMS += \
    screendialog.ui \
    mainwindow.ui \
    loadingdialog.ui

RESOURCES += \
    icon.qrc \
    icon.qrc

RC_FILE = icon.rc

CONFIG += console
