import QtQuick

Window {
    id: win
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Item {
        id: scene
        anchors.fill: parent

        Block {
            id: b1
            uid: 0
            Component.onCompleted: {
                Utils.registerSceneItem(b1)
            }
        }

        Reporter {
            id: b2
            uid: 1
            Component.onCompleted: {
                Utils.registerSceneItem(b2)
            }
        }

        Component.onCompleted: {
            Utils.init(scene)
        }
    }
}
