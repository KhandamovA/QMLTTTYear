import QtQuick

Item {
    id: root
    property bool isOutput: true
    property Item rootParent: null
    property bool candidate: false
    property bool busy: false
    readonly property bool isBlockConnector: true
    height: 24

    Rectangle {
        width: root.width
        height: root.height
        border.color: root.isOutput ? "blue" : "orange"
        border.width: 0
        color: root.candidate ? "blue" : "transparent"
        opacity: root.candidate ? 0.5 : 1.0
    }
}
