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
            Component.onCompleted: {
                Utils.registerSceneItem(b1)
            }
        }

        Reporter {
            id: b2
            uid: 2
            Component.onCompleted: {
                Utils.registerSceneItem(b2)
            }
        }

        Reporter {
            id: b5
            uid: 7
            Component.onCompleted: {
                Utils.registerSceneItem(b5)
            }
        }

        Reporter {
            id: b3
            uid: 3
            Component.onCompleted: {
                Utils.registerSceneItem(b3)
            }
        }

        Block {
            id: b4
            uid: 4
            viewTexts: ["4 Установить $$", "and $$"]
            Component.onCompleted: {
                Utils.registerSceneItem(b4)
            }
        }
        Block {
            id: b7
            uid: 8
            viewTexts: ["8 Установить $$"]
            Component.onCompleted: {
                Utils.registerSceneItem(b7)
            }
        }

        Block {
            id: b9
            uid: 9
            viewTexts: ["9 Установить $$", "and $$"]
            Component.onCompleted: {
                Utils.registerSceneItem(b9)
            }
        }
        Block {
            id: b10
            uid: 10
            viewTexts: ["10 Установить $$", "and $$"]
            Component.onCompleted: {
                Utils.registerSceneItem(b10)
            }
        }

        Component.onCompleted: {
            Utils.init(scene)
        }
    }
}
