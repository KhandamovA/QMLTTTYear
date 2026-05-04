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

            Component.onCompleted: {
                if (contentItem) {
                    contentItem.maximumFlickVelocity = 0
                    contentItem.flickDeceleration = 0
                }
            }

            WheelHandler {
                // Указываем, что обработчик работает для контента ScrollView
                target: scrollView.contentItem

                // Настраиваем скорость (чем выше, тем быстрее)
                rotationScale: 5.0

                // Настраиваем ориентацию (вертикально)
                orientation: Qt.Vertical

                // Важный момент: чтобы скролл физически двигался,
                // WheelHandler должен менять contentY через свойство point.
                // Но в QML проще всего это сделать через обработчик onWheel:
                onWheel: event => {
                    let speed = 100
                    // Базовая скорость (пиксели)
                    let delta = event.angleDelta.y > 0 ? -speed : speed

                    let newY = scrollView.contentItem.contentY + delta

                    // Ограничиваем в пределах контента
                    let maxY = scrollView.contentItem.contentHeight - scrollView.contentItem.height
                    scrollView.contentItem.contentY = Math.max(0, Math.min(newY, maxY))
                }
            }
        }
    }

    Component.onCompleted: {}
}
