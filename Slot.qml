import QtQuick

Rectangle {
    id: root
    width: Math.max(26, input.width + (root.margins * 2))
    height: 18
    radius: root.width / 2
    antialiasing: true

    border.color: "black"
    border.width: 0
    color: "white"

    property int margins: 4

    TextInput {
        id: input
        x: root.margins
        height: root.height
        text: ""
        width: implicitWidth > 18 ? implicitWidth : 18
    }
}
