import QtQuick

Window {
    id: win
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Scene {
        id: scene
        anchors.fill: parent

        Block {
            id: b1
            viewTexts: ["1 Установить $$", "and $$"]
        }

        Reporter {
            id: b2
        }

        Reporter {
            id: b5
        }

        Reporter {
            id: b3
        }

        Block {
            id: b4
            viewTexts: ["4 Установить $$", "and $$"]
        }
        Block {
            id: b7
            viewTexts: ["8 Установить $$"]
        }

        Block {
            id: b9
            viewTexts: ["9 Установить $$", "and $$"]
        }
        Block {
            id: b10
            hasOutput: false
            viewTexts: ["10 Установить $$"]
        }

        Component.onCompleted: {
            Utils.init(scene.containter)
            Utils.registerSceneItem(b10)
            Utils.registerSceneItem(b9)
            Utils.registerSceneItem(b1)
            Utils.registerSceneItem(b2)
            Utils.registerSceneItem(b3)
            Utils.registerSceneItem(b4)
            Utils.registerSceneItem(b5)
            Utils.registerSceneItem(b7)
        }
    }
}
