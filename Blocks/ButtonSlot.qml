import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property int margins: 6
    property int index: 0

    property var rootParent: null
    property var ownerBlock: null
    readonly property bool isButtonSlot: true
    property var currentValue: ({})

    height: 20
    // Ширина подстраивается под текст, но не меньше минимальной
    width: Math.max(30, input.implicitWidth + (margins * 2))

    // Идеальный радиус для капсулы — половина высоты
    radius: height / 2

    // Сглаживание включаем, но убираем слои
    antialiasing: true
    layer.enabled: false

    // Цвета из твоей логики

    // Внутренняя часть (тело слота)
    Rectangle {
        anchors.fill: parent
        anchors.margins: !root.busy ? (root.candidate ? 2 : 1) : 0
        color: "white"
        radius: parent.radius - 1 // Чуть меньше, чтобы не просвечивало
        antialiasing: true
    }

    Text {
        id: input
        anchors.centerIn: parent // Центрируем по всему овалу
        text: currentValue.key ? currentValue.key : "Выбрать..."
        font.pixelSize: 12
        font.italic: true
        verticalAlignment: TextInput.AlignVCenter
        horizontalAlignment: TextInput.AlignHCenter
        color: "#333333"
        width: implicitWidth > 18 ? implicitWidth : 18
        visible: !root.busy
    }

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            let newValue = Utils.qmlQuery("buttonSetter", {
                "type": ownerBlock.type,
                "index": root.index,
                "key": currentValue.key,
                "value": currentValue.value,
                "blockUid": ownerBlock.uid
            })

            root.currentValue = newValue
        }
    }
}
