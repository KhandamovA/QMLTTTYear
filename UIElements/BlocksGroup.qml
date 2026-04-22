import QtQuick
import QtQuick.Controls

Item {
    id: groupRoot
    property alias column: column
    property string name: ""
    property string textColor: "black"
    property string bodyColor: "#bfcdd9"
    property bool isOpen: true

    // Используем implicit размеры, чтобы Item правильно занимал место в родителе
    width: mainCol.implicitWidth
    height: mainCol.implicitHeight

    Column {
        id: mainCol
        spacing: 5
        width: parent.width // Чтобы колонка знала ширину родителя

        // Заголовок
        Rectangle {
            id: header
            // Ширина заголовка: текст + отступ + стрелочка + запас
            width: column.implicitWidth + 14
            height: groupTitle.implicitHeight + 14
            color: groupRoot.isOpen ? Qt.darker(groupRoot.bodyColor, 1.3) : groupRoot.bodyColor
            radius: 6

            // Обертка для текста и стрелочки, чтобы легко рулить отступами
            Row {
                anchors.centerIn: parent
                spacing: 15 // Вот этот отступ между текстом и треугольником

                Text {
                    id: groupTitle
                    text: groupRoot.name
                    color: groupRoot.textColor
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: groupRoot.isOpen = !groupRoot.isOpen
            }
        }

        // Контейнер с анимацией
        Item {
            id: clipper
            width: column.width
            // Только анимация высоты
            height: groupRoot.isOpen ? column.implicitHeight : 0
            clip: true

            Behavior on height {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }

            Column {
                id: column

                spacing: 10
                leftPadding: 10
                topPadding: 5

                // Opacity убрали, теперь блоки просто "срезаются" краем clipper
            }
        }
    }
}
