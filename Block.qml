import QtQuick
import QtQuick.Shapes

Item {
    id: root

    property string viewText: "Если $$ тогда"
    property bool hasInput: true
    property string textColor: "black"
    property string bodyColor: "#cccccc"

    Item {
        id: props

        property var polyPath: []
        property int margins: 8
        property int arrowMargin: margins + 5
        property int arrowWidth: 16
        property int arrowHeight: 12

        property int containersMargin: 16
        property int minWidth: 64
        property int actualWidth: view.width + (margins * 2)
        property int actualHeight: view.height + (margins * 2) + arrowHeight

        onActualWidthChanged: {
            console.log(actualWidth)
            props.updatePolyPath()
        }

        function drawInputFromLeft(beginPoint) {
            return [Qt.point(beginPoint.x, beginPoint.y), Qt.point(beginPoint.x, beginPoint.y + props.arrowHeight), Qt.point(beginPoint.x + props.arrowWidth, beginPoint.y + props.arrowHeight), Qt.point(beginPoint.x + props.arrowWidth, beginPoint.y)]
        }

        function drawInputFromRight(beginPoint) {
            return [Qt.point(beginPoint.x, beginPoint.y), Qt.point(beginPoint.x, beginPoint.y + props.arrowHeight), Qt.point(beginPoint.x - props.arrowWidth, beginPoint.y + props.arrowHeight), Qt.point(beginPoint.x - props.arrowWidth, beginPoint.y)]
        }

        function updatePolyPath() {
            let temp = []

            let firstWidth = 0
            let lastWidth = 0

            if (containers.children.length > 2) {
                let f = containers.children[0]
                let l = containers.children[containers.children.length - 2]
                firstWidth = f.width
                lastWidth = l.width
            }

            //==================

            let items = []
            let totalHeight = 0
            let chlds = containers.children

            for (let i = 0; i < chlds.length; i++) {
                let cntnr = chlds[i]
                let polyPath = cntnr.polyPath

                if (polyPath) {
                    items.push({
                        "h": cntnr.height,
                        "path": polyPath
                    })
                    totalHeight += cntnr.height
                }
            }

            totalHeight = actualHeight

            for (let i = 0; i < items.length; i++) {
                let polyPath = items[i].path
                let h = items[i].h;
                // totalHeight -= h

                if (polyPath) {
                    for (let j = 0; j < polyPath.length; j++) {
                        let point = polyPath[j]
                        temp.push(Qt.point(point.x + containersMargin, point.y + totalHeight))
                    }
                    totalHeight += h
                }
            }

            //==================

            if (items.length == 0) {
                temp.push(Qt.point(props.arrowMargin + props.arrowWidth + props.containersMargin, containers.height + props.actualHeight))
            }

            drawInputFromRight(Qt.point(props.arrowMargin + props.arrowWidth, containers.height + props.actualHeight)).forEach(p => {
                temp.push(p)
            })

            temp.push(Qt.point(0, containers.height + props.actualHeight))
            temp.push(Qt.point(0, 0))
            temp.push(Qt.point(props.arrowMargin, 0))

            if (root.hasInput) {
                drawInputFromLeft(Qt.point(props.arrowMargin, 0)).forEach(p => {
                    temp.push(p)
                })
            }

            temp.push(Qt.point(Math.max(props.actualWidth, props.minWidth), 0))
            temp.push(Qt.point(Math.max(props.actualWidth, props.minWidth), props.actualHeight))
            temp.push(Qt.point(props.arrowMargin + props.arrowWidth + props.containersMargin, props.actualHeight));

            //==================

            props.polyPath = temp
        }
    }

    Component.onCompleted: {
        props.updatePolyPath()
    }

    Shape {
        width: root.width
        height: root.height
        containsMode: Shape.FillContains

        ShapePath {
            strokeWidth: 1
            strokeColor: "#888888"
            fillColor: root.bodyColor

            PathPolyline {
                path: props.polyPath
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
        y: props.margins + props.arrowHeight
        color: root.textColor
    }

    Column {
        id: containers
        x: props.containersMargin
        y: props.actualHeight

        onChildrenChanged: {
            props.updatePolyPath()
        }

        Repeater {
            model: ["Иначе", ""]

            delegate: Container {
                viewText: modelData
                textColor: root.textColor
            }
        }
    }
}
