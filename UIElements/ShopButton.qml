import QtQuick 2.15

Rectangle {
    id: root

    // Свойства
    property string text: "Button"
    property var onClicked: function () {
        console.log("Clicked!")
    }

    // Размеры под текст + отступы
    width: label.implicitWidth + 24
    height: label.implicitHeight + 12
    radius: 4 // Скругление всегда выглядит лучше

    color: mouseArea.pressed ? "#eee" : (mouseArea.containsMouse ? "#f8f8f8" : "white")
    border.width: 1
    border.color: mouseArea.containsMouse ? "#3498db" : "#ccc"

    // Анимация цвета и масштаба
    Behavior on color {
        ColorAnimation {
            duration: 100
        }
    }
    Behavior on border.color {
        ColorAnimation {
            duration: 100
        }
    }

    // Эффект небольшого сжатия при нажатии
    scale: mouseArea.pressed ? 0.96 : 1.0
    Behavior on scale {
        NumberAnimation {
            duration: 50
            easing.type: Easing.OutQuad
        }
    }

    Text {
        id: label
        text: root.text
        color: mouseArea.containsMouse ? "#3498db" : "black"
        anchors.centerIn: parent
        font.pixelSize: 14

        Behavior on color {
            ColorAnimation {
                duration: 100
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true // Важно для работы containsMouse
        cursorShape: Qt.PointingHandCursor
        onClicked: root.onClicked()
    }
}
