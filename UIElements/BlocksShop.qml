import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

Item {
    id: root

    // property var model: [
    //     {
    //         "type": 0,
    //         "viewTexts": [],
    //         "hasInput": true,
    //         "hasOutput": true,
    //         "textColor": "black",
    //         "bodyColor": "#bfcdd9",
    //         "blockShape": 0 ///< 0 - обычный блок, 1 - репортер
    //     }
    // ]

    property var model: []
    property var blocksData: ({})
    property var standartItems: []

    function getBlock(origin, type) {
        for (let i of model) {
            if (i.origin === origin) {
                if (i.type === type) {
                    return i
                }
            }
        }
        return null
    }

    onModelChanged: {
        blocksData = ({})
        let previewComponent = Qt.createComponent("BlockPreview.qml")
        let groupComponent = Qt.createComponent("BlocksGroup.qml")
        let childs = groups.children
        for (let i of childs) {
            i.destroy()
        }

        let actualGroups = ({})
        let counter = 0
        let copyModel = [...model, ...standartItems]

        for (let i of copyModel) {
            counter++
            let groupName = i["group"]
            let bodyColor = i["bodyColor"]
            let keys = Object.keys(actualGroups)
            let currGroup = null
            let type = i["type"]

            blocksData[type] = i

            if (keys.includes(groupName)) {
                currGroup = actualGroups[groupName]
            } else {
                let group = groupComponent.createObject(groups, {
                    "name": groupName,
                    "bodyColor": bodyColor
                })

                actualGroups[groupName] = group
                currGroup = group
            }

            let column = currGroup.column
            let obj = previewComponent.createObject(column, {
                "blockData": i
            })
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Column {
            id: buttonsCol
            spacing: 10

            ShopButton {
                text: "Создать новый блок"
                onClicked: function () {
                    Utils.qmlQuery("createNewBlock", {})
                }
            }

            ShopButton {
                text: "Создать переменную"
                onClicked: function () {
                    let varName = Utils.qmlQuery("createNewVar", {})

                    if (varName === "")
                        return
                }
            }
            ShopButton {
                text: "Удалить переменную"
                onClicked: function () {
                    Utils.qmlQuery("deleteVar", {})
                }
            }
        }

        ScrollView {
            id: scrollView
            width: parent.width
            height: parent.height - buttonsCol.height
            clip: true

            Column {
                id: groups
                spacing: 12
            }
        }
    }

    Component.onCompleted: {

        // Инициализация стандартных блоков
        standartItems.push({
            "origin": 2,
            "type": 0,
            "viewTexts": ["Значение переменной ??"],
            "blockShape": 1,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 1,
            "viewTexts": ["Установить переменную ?? в значение $$"],
            "blockShape": 0,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 2,
            "viewTexts": ["Очистить массив ??"],
            "blockShape": 0,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 3,
            "viewTexts": ["Добавить в массив ?? значение $$"],
            "blockShape": 0,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 6,
            "viewTexts": ["Удалить элемент #$$ из массива ??"],
            "blockShape": 0,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 4,
            "viewTexts": ["Значение элемента #$$ из массива ??"],
            "blockShape": 1,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 7,
            "viewTexts": ["Позиция элемента $$ в массиве ??"],
            "blockShape": 1,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 5,
            "viewTexts": ["Кол-во элементов массива ??"],
            "blockShape": 1,
            "group": "Данные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })
    }
}
