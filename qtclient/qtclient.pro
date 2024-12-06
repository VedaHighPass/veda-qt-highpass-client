QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Location of SMTP Library
SMTP_LIBRARY_LOCATION = $$PWD/external/SmtpMime

win32:CONFIG(release, debug|release): LIBS += -L$$SMTP_LIBRARY_LOCATION/release/ -lSmtpMime2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$SMTP_LIBRARY_LOCATION/debug/ -lSmtpMime2
else:unix: LIBS += -L$$SMTP_LIBRARY_LOCATION -lSmtpMime


SOURCES += \
    checkboxdelegate.cpp \
    databasemanager.cpp \
    datalist.cpp \
    highpasswindow.cpp \
    main.cpp \
    mainwindow.cpp \
    sendemail.cpp

HEADERS += \
    checkboxdelegate.h \
    databasemanager.h \
    datalist.h \
    highpasswindow.h \
    mainwindow.h \
    sendemail.h

FORMS += \
    highpasswindow.ui \
    mainwindow.ui \
    sendemail.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
