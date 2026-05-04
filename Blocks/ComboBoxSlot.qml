pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import QtQuick.Controls

Item {
    id: root
    width: inputField.width
    height: inputField.height

    property var items: [
        {
            "key": "text",
            "value": 1000
        }
    ]

    property var updateItemMethod: ({})

    property var currentValue: ({})
    property int maxItemWidth: 0

    property var rootParent: null
    property var ownerBlock: null

    readonly property bool isComboBoxSlot: true

    // Функция для восстановления последнего значения
    function restoreLastValue() {
        if (inputField) {
            inputField.text = currentValue.key ? currentValue.key : ""
            inputField.cursorPosition = 0
        }
    }

    // Функция для вычисления максимальной ширины
    function calculateMaxWidth() {
        var maxWidth = 0
        var tempText = Qt.createQmlObject('import QtQuick 2.15; Text { font.pixelSize: 12 }', root)
        for (var i = 0; i < items.length; i++) {
            tempText.text = items[i].key
            var textWidth = tempText.width + 40
            // +24 на отступы
            if (textWidth > maxWidth) {
                maxWidth = textWidth
            }
        }
        tempText.destroy()
        maxItemWidth = maxWidth
    }

    Component.onCompleted: {
        calculateMaxWidth()
    }

    // 1. Поле ввода с кнопкой
    TextField {
        id: inputField
        placeholderText: text === "" ? "Поиск..." : ""
        text: ""
        visible: true
        z: 1
        width: implicitWidth < 250 ? implicitWidth : 250

        rightPadding: 26
        leftPadding: 5
        verticalAlignment: TextInput.AlignVCenter
        font.pixelSize: 12

        background: Rectangle {
            border.color: inputField.activeFocus ? "#3498db" : "#bdc3c7"
            border.width: 1
            radius: inputField.width
        }

        ToolTip.text: inputField.text
        ToolTip.delay: 300
        ToolTip.visible: textFieldArea.containsMouse && inputField.text !== ""

        onTextChanged: {
            if (activeFocus) {
                popup.open()
            }
        }

        MouseArea {
            id: textFieldArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            propagateComposedEvents: true  // Позволяет событиям проходить дальше к TextField

            onClicked: function (mouse) {
                // Не поглощаем событие, передаем его дальше
                inputField.focus = true
                inputField.selectAll()
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 8
            width: 14
            height: 20
            radius: 14
            // color: area.pressed ? "#e0e0e0" : (area.containsMouse ? "#f0f0f0" : "transparent")

            Shape {
                id: canvas
                anchors.centerIn: parent
                width: 10
                height: 6

                antialiasing: true
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    fillColor: "#7f8c8d"
                    strokeColor: "transparent"

                    // Рисуем треугольник носиком вниз
                    startX: 0
                    startY: 0
                    PathLine {
                        x: canvas.width
                        y: 0
                    }
                    PathLine {
                        x: canvas.width / 2
                        y: canvas.height
                    }
                    PathLine {
                        x: 0
                        y: 0
                    }
                }

                // Вместо смены координат в onPaint, просто вращаем Shape
                rotation: popup.opened ? 180 : 0

                Behavior on rotation {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            MouseArea {
                id: area
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (popup.opened) {
                        popup.close()
                    } else {
                        if (inputField.text !== "") {
                            inputField.text = ""
                        }
                        popup.open()
                        inputField.forceActiveFocus()
                    }
                }
            }
        }
    }

    // 2. Выпадающее окно
    Popup {
        id: popup
        y: inputField.height + 2
        width: Math.max(inputField.width, root.maxItemWidth)
        implicitHeight: Math.min(contentItem.implicitHeight, 200)
        padding: 1
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        onClosed: {
            if (inputField) {
                inputField.focus = false
            }
            root.restoreLastValue()
        }

        onOpened: {
            // Пересчитываем ширину при открытии на случай изменения items и получаем новые данные через колбэк
            root.items = root.updateItemMethod()
            root.calculateMaxWidth()
        }

        contentItem: ListView {
            id: listView
            clip: true
            implicitHeight: contentHeight
            width: popup.width

            model: {
                let searchText = inputField ? inputField.text.toLowerCase() : ""
                let filtered = root.items.filter(function (item) {
                    return item["key"].toLowerCase().indexOf(searchText) !== -1
                })
                return filtered
            }

            delegate: ItemDelegate {
                required property var modelData
                required property int index
                width: listView.width

                contentItem: Text {
                    text: modelData.key
                    font.pixelSize: 12
                    color: parent.enabled ? parent.palette.text : parent.palette.disabledText
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 8
                    rightPadding: 8
                }

                highlighted: listView.currentIndex === index

                onClicked: {
                    root.currentValue = modelData
                    popup.close()
                }
            }

            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            border.color: "#bdc3c7"
            border.width: 1
            radius: 4
        }
    }
}
