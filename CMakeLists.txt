cmake_minimum_required(VERSION 3.16)

set(ThisProjectName LogicFactory)
set(ThisLibName ${ThisProjectName}_lib)
set(ThisExampleName ${ThisProjectName}_example)

project(${ThisProjectName} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup(REQUIRES 6.8)



set_source_files_properties(Blocks/Utils.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

set_source_files_properties(Blocks/Resources.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)


qt_add_qml_module(${ThisLibName}
    URI tttYear
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml

    QML_FILES

    Blocks/Scene.qml
    Blocks/Block.qml
    Blocks/Container.qml
    Blocks/BlockTitle.qml
    Blocks/Slot.qml
    Blocks/Reporter.qml
    Blocks/Utils.qml
    Blocks/BlockConnector.qml
    Blocks/ButtonSlot.qml
    Blocks/ReplicaSlot.qml

    UIElements/BlocksShop.qml
    UIElements/BlockPreview.qml
    UIElements/BlocksGroup.qml
    UIElements/ConstructorScene.qml
    UIElements/ShopButton.qml
    Blocks/ComboBoxSlot.qml
    Blocks/Resources.qml
    Blocks/Main.qml

    SOURCES
    Integration/editorwatcher.h Integration/editorwatcher.cpp
    Integration/editorscene.h Integration/editorscene.cpp
    Integration/blockeditor.h Integration/blockeditor.cpp
    Integration/datacontext.h Integration/datacontext.cpp
    Integration/dialogwithlist.h Integration/dialogwithlist.cpp
    Workflow/baseworkflow.h Workflow/baseworkflow.cpp
    Workflow/blockexecuter.h Workflow/blockexecuter.cpp
    Workflow/standartexecuters.h
    Workflow/runexecuter.h Workflow/runexecuter.cpp
    SOURCES Workflow/standartexecuters.cpp
)

target_include_directories(${ThisLibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Integration")

set_source_files_properties(
    Blocks/Scene.qml
    Blocks/Block.qml
    Blocks/Container.qml
    Blocks/BlockTitle.qml
    Blocks/Slot.qml
    Blocks/Reporter.qml
    Blocks/Utils.qml
    Blocks/BlockConnector.qml
    Blocks/ComboBoxSlot.qml
    Blocks/Resources.qml
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

target_link_libraries(${ThisLibName}
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Widgets
)

qt_add_executable(${ThisExampleName}
    main.cpp
)

target_link_libraries(${ThisExampleName}
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Widgets
    PRIVATE ${ThisLibName}
)

