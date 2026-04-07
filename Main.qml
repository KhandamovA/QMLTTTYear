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
        }

        Reporter {
            id: b2
            uid: 1
        }

        Component.onCompleted: {
            Utils.init(scene)
            Utils.registerSceneItem(b1)
            Utils.registerSceneItem(b2)
        }
    }
}
