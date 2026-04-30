import QtQuick
import QtQuick.Controls
import "../UIElements"

Rectangle {
    id: root

    property int margins: 6
    property int index: 0

    property var rootParent: null
    property var ownerBlock: null
    readonly property bool isReplicaSlot: true
    property var blockData: ({})

    height: input.height
    // Ширина подстраивается под текст, но не меньше минимальной
    width: input.width

    // Идеальный радиус для капсулы — половина высоты
    radius: height / 2

    // Сглаживание включаем, но убираем слои
    antialiasing: true
    layer.enabled: false

    BlockPreview {
        id: input
        blockData: root.blockData
    }
}
