import QtQuick 2.15
import "../Blocks"

Item {
    id: root
    scale: 1
    property var watcher: null
    property var interactive: true
    property var rootParent: ({
            "ZoomScale": 100
        })

    property int blockShape: 0

    onWatcherChanged: {
        // Кладем в синглтон, чтобы иметь доступ везде
        Utils.watcher = root.watcher
    }

    Component.onCompleted: {
        Utils.init(root, null)
    }

    Item {
        id: scene
        anchors.fill: root

        Reporter {
            id: block2
            anchors {
                left: parent.left
                leftMargin: 10  // если нужен отступ
                verticalCenter: parent.verticalCenter
            }
            visible: root.blockShape === 1
        }

        Block {
            id: block1
            anchors {
                left: parent.left
                leftMargin: 10  // если нужен отступ
                verticalCenter: parent.verticalCenter
            }
            visible: root.blockShape === 0
        }
    }

    Connections {
        target: root.watcher

        function onQml_signal(method: string, data: variant, signalId) {
            if (method == "setConstructorBlock") {
                root.blockShape = data.blockShape

                if (root.blockShape === 0) {
                    block1.viewTexts = data.viewTexts
                    block1.bodyColor = data.bodyColor
                    block1.textColor = data.textColor
                } else {
                    block2.viewText = data.viewTexts[0]
                    block2.bodyColor = data.bodyColor
                    block2.textColor = data.textColor
                }
            }
            // Обязательный вызов, перед выходом из функции иначе зависнет
            root.watcher.handleResponse({}, signalId)
        }
    }
}
