pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

// Layout.fillWidth: true
// Layout.fillHeight: true
// Layout.row: 0
// Layout.column: 0
// Layout.preferredWidth: 100
// Layout.rowSpan: 1
// Layout.columnSpan: 1

Rectangle {
    id: win
    width: 640
    height: 480
    visible: true

    property var watcher: null

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal  // Горизонтальное расположение панелей

        // --- Левая панель (магазин блоков) ---
        BlocksShop {
            id: blocksShop
            // Задаём минимальную и предпочтительную ширину
            SplitView.minimumWidth: 250
            Layout.preferredWidth: 100
            // Элемент НЕ будет растягиваться
            Layout.fillWidth: false
        }

        // --- Правая панель (сцена) ---
        Scene {
            id: scene
            // Это элемент, который будет занимать всё оставшееся место
            Layout.fillWidth: true
            // Можно также задать минимальную ширину
            Layout.minimumWidth: 200
            Layout.preferredWidth: 200
        }

        Component.onCompleted: {
            Utils.init(scene.containter, blocksShop)
        }
    }

    Item {
        id: helper

        function printValues(data, prefix = "") {
            let keys = Object.keys(data)
            for (let i of keys) {
                if (data[i] instanceof Object) {
                    console.log(i, ":")
                    printValues(data[i], prefix + " ")
                    console.log("")
                } else {
                    console.log(prefix + "{", i, ":", data[i], "}")
                }
            }
        }
    }

    Connections {
        target: win.watcher

        function onQml_signal(method: string, data: variant) {
            // console.log("call:", method)
            // helper.printValues(data)
            // console.log("")

            if (method == "registerBlock") {
                let temp = blocksShop.model
                blocksShop.model = []
                temp.push(data)
                blocksShop.model = temp
            }

            // Обязательный вызов, перед выходом из функции иначе зависнет
            win.watcher.handleResponse({})
        }
    }
}
