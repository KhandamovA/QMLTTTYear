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
            uid: 1
            viewTexts: ["1 Установить $$", "and $$"]
        }

        Reporter {
            id: b2
            uid: 2
        }

        Reporter {
            id: b5
            uid: 7
        }

        Reporter {
            id: b3
            uid: 3
        }

        Block {
            id: b4
            uid: 4
            viewTexts: ["4 Установить $$", "and $$"]
        }
        Block {
            id: b7
            uid: 8
            viewTexts: ["8 Установить $$"]
        }

        Block {
            id: b9
            uid: 9
            viewTexts: ["9 Установить $$", "and $$"]
        }
        Block {
            id: b10
            uid: 10
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
