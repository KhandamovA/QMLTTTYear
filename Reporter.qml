import QtQuick
import QtQuick.Shapes

Item {
    id: root

    property string viewText: "Значение переменной $$"
    property string textColor: "black"
    property string bodyColor: "#cccccc"

    Item {
        id: props

        property int margins: 6
        property int arrowWidth: 16
        property int arrowHeight: 12
    }

    Shape {
        id: render
        width: view.width + props.margins * 2
        height: view.height + props.margins * 2
        containsMode: Shape.FillContains
        antialiasing: true

        layer.enabled: true
        layer.samples: 8
        layer.smooth: true

        ShapePath {
            strokeWidth: 1
            strokeColor: "#888888"
            fillColor: root.bodyColor
            capStyle: ShapePath.RoundCap

            PathRectangle {
                x: 0
                y: 0
                width: render.width
                height: render.height
                radius: width / 2
            }
        }

        TapHandler {
            id: tapHandler
            onTapped: console.log("Работает!")
        }

        DragHandler {
            id: dragHandler
            target: root // Перемещаем весь корневой объект
            cursorShape: Qt.SizeAllCursor // Меняем курсор при наведении
        }
    }

    BlockTitle {
        id: view
        viewText: root.viewText
        x: props.margins
        y: props.margins
        color: root.textColor
    }
}
