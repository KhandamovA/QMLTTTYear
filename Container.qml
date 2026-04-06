import QtQuick
import QtQuick.Shapes

Item {
    id: root

    property string viewText: "test text"
    property string textColor: "white"

    width: view.width + (margins * 2)
    height: view.height + (margins * 2) + arrowHeight + container.actualHeight

    property var polyPath: []

    property int margins: 8
    property int arrowMargin: margins + 5
    property int arrowWidth: 16
    property int arrowHeight: 12

    property int containersMargin: 16
    property int minWidth: 64

    function drawInputFromLeft(beginPoint) {
        return [Qt.point(beginPoint.x, beginPoint.y), Qt.point(beginPoint.x, beginPoint.y + root.arrowHeight), Qt.point(beginPoint.x + root.arrowWidth, beginPoint.y + root.arrowHeight), Qt.point(beginPoint.x + root.arrowWidth, beginPoint.y)]
    }

    function drawInputFromRight(beginPoint) {
        return [Qt.point(beginPoint.x, beginPoint.y), Qt.point(beginPoint.x, beginPoint.y + root.arrowHeight), Qt.point(beginPoint.x - root.arrowWidth, beginPoint.y + root.arrowHeight), Qt.point(beginPoint.x - root.arrowWidth, beginPoint.y)]
    }

    Component.onCompleted: {
        let temp = [];

        // temp.push(Qt.point(view.width + (margins * 2), 0))
        temp.push(Qt.point(arrowMargin + arrowWidth, 0))

        drawInputFromRight(Qt.point(arrowMargin + arrowWidth, 0)).forEach(p => {
            temp.push(p)
        })
        temp.push(Qt.point(0, 0))
        temp.push(Qt.point(0, container.actualHeight))
        temp.push(Qt.point(arrowMargin, container.actualHeight))

        drawInputFromLeft(Qt.point(arrowMargin, container.actualHeight)).forEach(p => {
            temp.push(p)
        })

        temp.push(Qt.point(Math.max(view.width + (margins * 2), minWidth), container.actualHeight))
        temp.push(Qt.point(Math.max(view.width + (margins * 2), minWidth), container.actualHeight + view.height + arrowHeight + (margins * 2)))
        temp.push(Qt.point(arrowMargin + arrowWidth, container.actualHeight + view.height + arrowHeight + (margins * 2)))

        polyPath = temp
    }

    Shape {
        width: root.width
        height: root.height

        ShapePath {
            strokeWidth: 1
            strokeColor: "black"
            fillColor: "transparent"

            // PathPolyline {
            //     path: root.polyPath
            // }
        }
    }

    Item {
        id: container
        property int actualHeight: height < 1 ? 24 : height
    }

    BlockTitle {
        id: view
        viewText: root.viewText
        x: root.margins
        y: root.margins + root.arrowHeight + container.actualHeight
        color: root.textColor
    }
}
