cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(ThisProjectName LogicFactory)
set(ThisLibName ${ThisProjectName}_lib)
set(ThisExampleName ${ThisProjectName}_example)

project(${ThisProjectName} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # Явно укажем стандарт, как и в верхнем проекте
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets)

qt_standard_project_setup(REQUIRES 6.8)

# ==============================================================================
# 1. СНАЧАЛА УСТАНАВЛИВАЕМ СВОЙСТВА ФАЙЛОВ (До qt_add_qml_module!)
# ==============================================================================

# Установка синглтонов
set_source_files_properties(Blocks/Utils.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)
set_source_files_properties(Blocks/Resources.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

# # Установка алиасов ресурсов
# set_source_files_properties(
#     Blocks/Scene.qml
#     Blocks/Block.qml
#     Blocks/Container.qml
#     Blocks/BlockTitle.qml
#     Blocks/Slot.qml
#     Blocks/Reporter.qml
#     Blocks/Utils.qml
#     Blocks/BlockConnector.qml
#     Blocks/ComboBoxSlot.qml
#     Blocks/Resources.qml
#     PROPERTIES QT_RESOURCE_ALIAS "Blocks/" # Лучше использовать относительный путь внутри QRC, а не абсолютный системный
# )

# set_source_files_properties(
#     UIElements/BlocksShop.qml
#     PROPERTIES QT_RESOURCE_ALIAS "UIElements/BlocksShop.qml"
# )

# ПРИМЕЧАНИЕ: QT_RESOURCE_ALIAS для .cpp и .h файлов (editorwatcher и т.д.) делать НЕ нужно.
# Они компилируются в C++ код, а не упаковываются в QRC как бинарные ресурсы.

# ==============================================================================
# 2. ТЕПЕРЬ СОЗДАЕМ QML МОДУЛЬ
# ==============================================================================

qt_add_qml_module(${ThisLibName}
    URI ${ThisProjectName}
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
    Workflow/standartexecuters.h Workflow/standartexecuters.cpp
    Workflow/runexecuter.h Workflow/runexecuter.cpp
    SOURCES
)

target_include_directories(${ThisLibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Integration")

target_link_libraries(${ThisLibName}
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Widgets
)

# ==============================================================================
# 3. ПРОЕКТ-ПРИМЕР
# ==============================================================================

qt_add_executable(${ThisExampleName}
    main.cpp
    mydatacontext.h mydatacontext.cpp
)

target_link_libraries(${ThisExampleName}
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Widgets
    PRIVATE ${ThisLibName}
)