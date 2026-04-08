import QtQuick
import QtQuick.Shapes

Item {
    id: root
    property Item rootParent: null

    property int uid: -1

    property string viewText: "Значение переменной $$ $$"
    property string textColor: "black"
    property string bodyColor: "#cccccc"

    width: shape.width
    height: shape.height

    property var objectsGridPos: ({})
    property alias shape: shape

    Item {
        id: props

        property int margins: 6
        property int arrowWidth: 16
        property int arrowHeight: 12
    }

    Shape {
        id: shape
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
                width: shape.width
                height: shape.height
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

            onActiveChanged: {
                if (!active) {
                    if (root.x < 0)
                        root.x = 0

                    if (root.y < 0)
                        root.y = 0

                    Utils.changeGridPos(root)
                    view.updateSlotsData()
                } else {
                    Utils.raise(root)
                }
            }

            onCentroidChanged: {
                // console.log(root.x, root.y)
                let items = Utils.getItemsForGrid(root.x, root.y);

                // console.log(items.length)
            }
        }
    }

    BlockTitle {
        id: view
        rootParent: root
        viewText: root.viewText
        x: props.margins
        y: props.margins
        color: root.textColor
    }
}
