cmake_minimum_required(VERSION 3.16)

project(demo3_QT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( Qt6  REQUIRED COMPONENTS Widgets Network Multimedia )
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network )

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        bookeditdialog.cpp
        bookeditdialog.h
        bookeditdialog.ui

        bookdata.h

        bookmanager.cpp
        bookmanager.h

        readingview.h
        readingview.cpp
        readingview.ui
        readerwindow.h
        readerwindow.cpp
        readerwindow.ui


        sidenavigationview.cpp
        sidenavigationview.h
        sideNavigationview.ui


        # 资源文件 (.qrc) - 使用正确的路径！
        picture.qrc
)

# --- 创建可执行文件 ---
# 不论Qt版本是5还是6，都使用这个简洁的命令
add_executable(demo3_QT ${PROJECT_SOURCES}
        bookeditdialog.h bookeditdialog.cpp bookeditdialog.ui
        readingview.ui
        readingview.h readingview.cpp
        readerwindow.h readerwindow.cpp readerwindow.ui
        settingsmanager.h settingsmanager.cpp
        pictures.qrc
        bookmarkstyledialog.h bookmarkstyledialog.cpp bookmarkstyledialog.ui
        dictionaryservice.h dictionaryservice.cpp
        tooltippopup.h tooltippopup.cpp tooltippopup.ui
        data.qrc
        ttsplayer.h ttsplayer.cpp ttsplayer.ui
        timerwindow.h timerwindow.cpp
        configmanager.h configmanager.cpp
        keyworddialog.h keyworddialog.cpp
        keywordhighlighter.h keywordhighlighter.cpp
        keywordmanager.h keywordmanager.cpp
        notepadwidget.h
        style.qrc
        style.qss



)
# if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#     qt_add_executable(demo3_QT
#         MANUAL_FINALIZATION
#         ${PROJECT_SOURCES}

#     )
# # Define target properties for Android with Qt 6 as:
# #    set_property(TARGET demo3_QT APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
# #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
# else()
#     if(ANDROID)
#         add_library(demo3_QT SHARED
#             ${PROJECT_SOURCES}
#         )
# # Define properties for Android with Qt 5 after find_package() calls as:
# #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#     else()
#         add_executable(demo3_QT
#             ${PROJECT_SOURCES}
#         )
#     endif()
# endif()
target_include_directories(demo3_QT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(demo3_QT PRIVATE Qt::Widgets Qt::Network Qt::Multimedia)

#这一段是原来就删了的
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.



# if(${QT_VERSION} VERSION_LESS 6.1.0)
#   set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.demo3_QT)
# endif()
# set_target_properties(demo3_QT PROPERTIES
#     ${BUNDLE_ID_OPTION}
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#     MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE TRUE
# )

# include(GNUInstallDirs)
# install(TARGETS demo3_QT
#     BUNDLE DESTINATION .
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )

# --- 设置特定于平台的属性 (例如 Windows, macOS) ---
set_target_properties(demo3_QT PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(demo3_QT)
endif()
