pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "./Blocks"

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

    onWatcherChanged: {
        // Кладем в синглтон, чтобы иметь доступ везде
        Utils.watcher = win.watcher
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal  // Горизонтальное расположение панелей

        handle: Rectangle {
            implicitWidth: 8       // Ширина тянучки
            color: "#cccccc"

            Rectangle {
                width: 2
                height: 30
                color: "#999999"
                anchors.centerIn: parent
            }
        }

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

        function onQml_signal(method: string, data: variant, signalId: int) {
            // console.log("call:", method)
            // helper.printValues(data)
            // console.log("")

            if (method == "registerBlock") {
                let data_ = data.data
                let checkDefine = data.checkDefine
                let origin = data_["origin"]

                let temp = blocksShop.model
                blocksShop.model = []
                if (origin === 2) {
                    blocksShop.standartItems.push(data_)
                } else {
                    temp.push(data_)
                }
                blocksShop.model = temp

                if (checkDefine) {
                    if (origin === 1) {
                        Utils.checkDefineForDynamicBlock(data_.type)
                    }
                }
            } else if (method == "saveScript") {
                let chains = Resources.sceneChainsToJson()
                let scenePosX = scene.contentX
                let scenePosY = scene.contentY
                let zoomScale = scene.zoomScale

                // console.log("posWithScale", scenePosX / zoomScale, scenePosY / zoomScale)

                win.watcher.handleResponse({
                    "chains": chains,
                    "scenePosX": scenePosX,
                    "scenePosY": scenePosY,
                    "zoomScale": zoomScale
                }, signalId)

                return
            } else if (method == "loadScript") {
                scene.zoomScale = data.zoomScale
                scene.contentX = data.scenePosX
                scene.contentY = data.scenePosY
                let chains = data.chains
                Resources.loadChainsToScene(chains)
            } else if (method == "clearScript") {
                Resources.clearScene();
                // Удаление всех динамических блоков
                Utils.blocksShop.deleteDynamicBlocks()
                // console.log("deleteDynamicBlocks")
            }

            // Обязательный вызов, перед выходом из функции иначе зависнет
            win.watcher.handleResponse({}, signalId)
        }
    }
}
