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
            id: b3
            uid: 3
            Component.onCompleted: {
                Utils.registerSceneItem(b3)
            }
        }

        Block {
            id: b4
            uid: 4
            Component.onCompleted: {
                Utils.registerSceneItem(b4)
            }
        }

        Component.onCompleted: {
            Utils.init(scene)
        }
    }
}
