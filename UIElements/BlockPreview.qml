import QtQuick 2.15
import QtQuick.Controls 2.15 // Для Overlay

Item {
    id: root

    // Сохраняем оригинальные данные отдельно, чтобы не портить их делейтами
    property var originalData: ({})
    property var blockData: ({})

    onBlockDataChanged: {
        originalData = blockData
        // Сохраняем для Drag
        updateImageFromData(blockData)
    }

    function updateImageFromData(data) {
        // 1. Делаем копию данных специально для рендеринга
        let renderData = Object.assign({}, data);

        // Извлекаем то, что нужно для логики выбора компонента
        let shape = renderData["blockShape"];

        // 2. УДАЛЯЕМ лишнее из копии, чтобы createObject не ругался
        delete renderData["blockShape"]
        delete renderData["group"]

        let componentPath = ""
        if (shape === 1) {
            componentPath = "../Blocks/Reporter.qml"
            // Специфика для репортера
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

        let container = Qt.createQmlObject('import QtQuick; Item { visible: false; opacity: 1 }', root);
        // 3. Создаем объект, передавая ОЧИЩЕННУЮ копию
        let obj = component.createObject(container, renderData)

        obj.x += 2
        obj.y += 2
        if (!obj) {
            console.error("Не удалось создать объект для рендера. Проверь оставшиеся свойства в renderData")
            return
        }

        obj.opacity = 1.0
        let targetSize = Qt.size(obj.width + 2, obj.height + 2)

        container.width = targetSize.width
        container.height = targetSize.height
        root.width = targetSize.width
        root.height = targetSize.height

        container.grabToImage(function (result) {
            previewImage.source = ""
            previewImage.source = result.url
            obj.destroy()
            container.destroy()
        }, targetSize)
    }

    // Основное изображение в магазине
    Image {
        id: previewImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            // Включаем Drag
            drag.target: ghostItem

            onPressed: {
                // Вычисляем начальную позицию призрака на экране
                let globalPos = previewImage.mapToItem(Overlay.overlay, 0, 0)
                ghostItem.x = globalPos.x
                ghostItem.y = globalPos.y

                ghostItem.Drag.active = true
            }

            onReleased: {
                ghostItem.Drag.drop()
                ghostItem.Drag.active = false
            }
        }
    }

    // Этот элемент будет летать ПОВЕРХ всего интерфейса
    Item {
        id: ghostItem
        parent: Overlay.overlay // Выносим в слой поверх всех окон
        width: root.width
        height: root.height
        visible: Drag.active

        // Свойства Drag
        Drag.active: mouseArea.drag.active
        Drag.keys: ["block"]
        // Передаем полные данные для сцены
        Drag.mimeData: {
            "blockData": root.originalData
        }
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        // Сама картинка, которая следует за мышью
        Image {
            anchors.fill: parent
            source: previewImage.source
            opacity: 0.8
        }
    }
}
