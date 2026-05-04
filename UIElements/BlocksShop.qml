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
    property var standartItems: []

    function getBlock(origin, type) {
        let copyModel = [...model, ...standartItems]
        for (let i of copyModel) {
            if (i.origin === origin) {
                if (i.type === type) {
                    return i
                }
            }
        }
        return null
    }

    function deleteDynamicBlock(type) {
        let temp = [...root.model]
        let filter = []
        for (let x of temp) {
            if (x.type === type && x.origin === 1) {
                continue
            }
            filter.push(x)
        }
        root.model = filter
    }

    function deleteDynamicBlocks() {
        let temp = [...root.model]
        temp = temp.filter(x => {
            return x.origin !== 1
        })
        root.model = temp
    }

    onModelChanged: {
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

            ShopButton {
                text: "Save"
                onClicked: function () {
                    let data = Utils.watcher.saveScript()
                    Utils.saveToFile("temp.json", data)
                }
            }

            ShopButton {
                text: "Load"
                onClicked: function () {
                    let data = Utils.loadFromFile("temp.json")

                    Utils.watcher.loadScript(data)
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

        // origin = 2 - Системные блоки
        // Инициализация стандартных блоков
        // ==================== 1. ОБЫЧНЫЕ ПЕРЕМЕННЫЕ (origin: 2 - переменная, но без индексов/ключей) ====================
        standartItems.push({
            "origin": 2,
            "type": 0 // Значение
            ,
            "viewTexts": ["Значение ??"],
            "blockShape": 1,
            "group": "Переменные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 1 // Установить
            ,
            "viewTexts": ["Установить ?? в значение $$"],
            "blockShape": 0,
            "group": "Переменные",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        });

        // ==================== 2. МАССИВЫ ====================
        standartItems.push({
            "origin": 2,
            "type": 2 // Очистить массив
            ,
            "viewTexts": ["Очистить массив ??"],
            "blockShape": 0,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 3 // Добавить в конец
            ,
            "viewTexts": ["Добавить в массив ?? значение $$"],
            "blockShape": 0,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 4 // Удалить по индексу
            ,
            "viewTexts": ["Удалить из массива ?? элемент #$$"],
            "blockShape": 0,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 5 // Значение по индексу
            ,
            "viewTexts": ["Значение элемента #$$ из массива ??"],
            "blockShape": 1,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 6 // Индекс по значению (позиция)
            ,
            "viewTexts": ["Индекс элемента $$ в массиве ??"],
            "blockShape": 1,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 7 // Количество элементов
            ,
            "viewTexts": ["Длина массива ??"],
            "blockShape": 1,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 8 // Заменить по индексу (новый тип)
            ,
            "viewTexts": ["Заменить в массиве ?? элемент #$$ на значение $$"],
            "blockShape": 0,
            "group": "Массивы",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        });

        // ==================== 3. MAP (Словари/Объекты) ====================
        standartItems.push({
            "origin": 2,
            "type": 9 // Установить по ключу
            ,
            "viewTexts": ["Установить в словарь ?? по ключу $$ значение $$"],
            "blockShape": 0,
            "group": "Словари",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 10 // Очистить словарь
            ,
            "viewTexts": ["Очистить словарь ??"],
            "blockShape": 0,
            "group": "Словари",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 11 // Количество ключей
            ,
            "viewTexts": ["Кол-во ключей в словаре ??"],
            "blockShape": 1,
            "group": "Словари",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 12 // Ключ по индексу
            ,
            "viewTexts": ["Ключ по индексу #$$ из словаря ??"],
            "blockShape": 1,
            "group": "Словари",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 13 // Значение по ключу
            ,
            "viewTexts": ["Значение по ключу $$ из словаря ??"],
            "blockShape": 1,
            "group": "Словари",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        standartItems.push({
            "origin": 2,
            "type": 14 // Удалить по ключу
            ,
            "viewTexts": ["Удалить из словаря ?? ключ $$"],
            "blockShape": 0,
            "group": "Словари",
            "bodyColor": "#bfcdd9",
            "textColor": "black"
        })

        let temp = model
        model = []
        temp = model
    }
}
