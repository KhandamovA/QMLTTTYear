cmake_minimum_required(VERSION 3.16)

project(tttYear VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(apptttYear
    main.cpp
)

set_source_files_properties(Utils.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(apptttYear
    URI tttYear
    QML_FILES
        Main.qml
        QML_FILES Block.qml
        QML_FILES Container.qml
        QML_FILES BlockTitle.qml
        QML_FILES Slot.qml
        QML_FILES Reporter.qml
        QML_FILES Utils.qml
        QML_FILES Scene.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apptttYear PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptttYear
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apptttYear
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS apptttYear
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
