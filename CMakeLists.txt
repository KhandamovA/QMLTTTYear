cmake_minimum_required(VERSION 3.16)

project(tttYear VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(apptttYear
    main.cpp
)

set_source_files_properties(Blocks/Utils.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

target_include_directories(apptttYear PRIVATE Integration)

qt_add_qml_module(apptttYear
    URI tttYear
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml

    QML_FILES
    Main.qml
    Blocks/Scene.qml
    Blocks/Block.qml
    Blocks/Container.qml
    Blocks/BlockTitle.qml
    Blocks/Slot.qml
    Blocks/Reporter.qml
    Blocks/Utils.qml
    Blocks/BlockConnector.qml

    UIElements/BlocksShop.qml

    SOURCES
    Integration/editorwatcher.h Integration/editorwatcher.cpp
    Integration/editorscene.h Integration/editorscene.cpp
    QML_FILES UIElements/BlockPreview.qml
    QML_FILES UIElements/BlocksGroup.qml
)

set_source_files_properties(
    Blocks/Scene.qml
    Blocks/Block.qml
    Blocks/Container.qml
    Blocks/BlockTitle.qml
    Blocks/Slot.qml
    Blocks/Reporter.qml
    Blocks/Utils.qml
    Blocks/BlockConnector.qml
    PROPERTIES QT_RESOURCE_ALIAS "${CMAKE_CURRENT_SOURCE_DIR}/Blocks/"
)

set_source_files_properties(
    UIElements/BlocksShop.qml
    PROPERTIES QT_RESOURCE_ALIAS "${CMAKE_CURRENT_SOURCE_DIR}/UIElements/"
)

set_source_files_properties(
    Integration/editorwatcher.h Integration/editorwatcher.cpp
    Integration/editorscene.h Integration/editorscene.cpp
    PROPERTIES QT_RESOURCE_ALIAS "${CMAKE_CURRENT_SOURCE_DIR}/Integration/"
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
target_link_libraries(apptttYear PRIVATE Qt6::Widgets)

include(GNUInstallDirs)
install(TARGETS apptttYear
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
