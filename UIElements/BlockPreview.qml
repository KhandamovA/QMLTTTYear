import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    property var originalData: ({})
    property var blockData: ({})
    property bool isDragging: false

    onBlockDataChanged: {
        originalData = blockData
        createPreviewObject()
    }

    // Живой превью-объект
    property var previewObject: null

    function createPreviewObject() {
        // Очищаем старый объект
        if (previewObject) {
            previewObject.destroy()
            previewObject = null
        }

        let renderData = Object.assign({}, blockData)
        let shape = renderData["blockShape"]

        delete renderData["blockShape"]
        delete renderData["group"]
        delete renderData["slotsPlaceHolders"]

        let componentPath = ""
        if (shape === 1) {
            componentPath = "../Blocks/Reporter.qml"
            renderData["viewText"] = renderData["viewTexts"][0]
            delete renderData["viewTexts"]
            delete renderData["hasInput"]
            delete renderData["hasOutput"]
        } else {
            componentPath = "../Blocks/Block.qml"
        }

        let component = Qt.createComponent(componentPath)

        if (component.status === Component.Error) {
            console.error("Ошибка загрузки:", component.errorString())
            return
        }

        // Создаем объект прямо в корне
        previewObject = component.createObject(root, renderData)

        if (!previewObject) {
            console.error("Не удалось создать объект")
            return
        }

        // Блокируем все интерактивные элементы для превью
        blockInteraction(previewObject);

        // Устанавливаем размеры корня под объект
        root.width = previewObject.width + 4
        root.height = previewObject.height + 4
        previewObject.x = 2
        previewObject.y = 2
        previewObject.shapePath.strokeWidth = 1;

        // Делаем полупрозрачным для визуального отличия
        previewObject.opacity = 0.9
    }

    // Рекурсивно блокируем все интерактивные элементы
    function blockInteraction(item) {
        if (!item)
            return

        // Блокируем MouseArea
        if (item.isInteractive !== undefined) {
            item.isInteractive = false
        }

        // Отключаем enabled у всех MouseArea и детей
        if (item.children) {
            for (let i = 0; i < item.children.length; i++) {
                let child = item.children[i]

                if (child.hasOwnProperty("enabled")) {
                    child.enabled = false
                }

                if (child.hasOwnProperty("acceptedButtons")) {
                    child.acceptedButtons = Qt.NoButton
                }

                // Рекурсивно обрабатываем детей
                if (child.children) {
                    blockInteraction(child)
                }
            }
        }
    }

    // Живой превью объект
    Item {
        id: previewContainer
        anchors.fill: parent

        // Пустой контейнер, объект добавляется динамически
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        preventStealing: true
        hoverEnabled: true
        drag.target: root.isDragging ? ghostItem : null

        onPressed: event => {
            if (!root.previewObject)
                return
            root.isDragging = true;

            // Вычисляем начальную позицию призрака
            let globalPos = mapToItem(Overlay.overlay, 0, 0)
            ghostItem.x = globalPos.x
            ghostItem.y = globalPos.y;

            // Создаем копию для перетаскивания
            root.createGhostCopy()

            ghostItem.Drag.active = true
            event.accepted = true
        }

        onReleased: {
            if (ghostItem.Drag.active) {
                ghostItem.Drag.drop()
                ghostItem.Drag.active = false
            }
            root.isDragging = false;

            // Удаляем временную копию
            if (ghostItem.dragObject) {
                ghostItem.dragObject.destroy()
                ghostItem.dragObject = null
            }
        }
    }

    function createGhostCopy() {
        if (!previewObject)
            return

        // Создаем точную копию данных
        let copyData = Object.assign({}, originalData)
        let shape = copyData["blockShape"]

        delete copyData["blockShape"]
        delete copyData["group"]
        delete copyData["slotsPlaceHolders"]

        let componentPath = shape === 1 ? "../Blocks/Reporter.qml" : "../Blocks/Block.qml"

        if (shape === 1) {
            copyData["viewText"] = copyData["viewTexts"][0]
            delete copyData["viewTexts"]
            delete copyData["hasInput"]
            delete copyData["hasOutput"]
        }

        let component = Qt.createComponent(componentPath)

        if (component.status === Component.Error) {
            console.error("Ошибка создания копии:", component.errorString())
            return
        }

        let copy = component.createObject(ghostItem, copyData)

        if (copy) {
            copy.opacity = 0.7
            copy.shapePath.strokeWidth = 1

            copy.scale = Utils.zoomScale

            ghostItem.dragObject = copy
            ghostItem.width = root.width
            ghostItem.height = root.height
        }
    }

    Item {
        id: ghostItem
        parent: Overlay.overlay
        visible: root.isDragging && Drag.active

        property Item dragObject: null

        Drag.active: mouseArea.drag.active
        Drag.keys: ["block"]
        Drag.mimeData: {
            "blockData": root.originalData
        }
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        // Отображаем живой объект вместо картинки
        Item {
            anchors.fill: parent
        }
    }

    // Следим за изменениями previewObject
    onPreviewObjectChanged: {
        if (previewObject) {
            previewObject.parent = previewContainer
        }
    }
}
