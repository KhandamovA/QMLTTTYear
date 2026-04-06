import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Block {
        uid: 0
    }

    Reporter {
        uid: 1
    }

    Component.onCompleted: {}
}
