import QtQuick
import QtQuick.Shapes

Item {
    id: root
    property Item rootParent: null

    property string viewText: "test text"
    property string textColor: "white"

    property var nextBlock: null
    readonly property bool isContainer: true
    property alias output_: output_

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

    function updatePolyPath() {
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

        polyPath = temp;

        // rootParent.updateSceneInfo()
    }

    function setNextBlock(block) {
        if (block) {
            block.prevContainer = root
            root.nextBlock = block

            block.parent = root
            block.x = 0
            block.y = 0
            block.updateSceneInfo()
        } else {
            if (root.nextBlock) {
                let next = root.nextBlock
                let rect = Utils._rectFromScene(next)
                next.parent = Utils.sceneContainer
                next.prevContainer = null
                next.x = rect.x
                next.y = rect.y
                root.nextBlock = null
                next.updateSceneInfo()
            }
        }
        updatePolyPath()
    }

    Component.onCompleted: {
        updatePolyPath()
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
        property int actualHeight: root.nextBlock ? root.nextBlock.chainHeight : height < 1 ? 24 : container.height
    }

    BlockTitle {
        id: view
        rootParent: root
        viewText: root.viewText
        x: root.margins
        y: root.margins + root.arrowHeight + container.actualHeight
        color: root.textColor

        onWidthChanged: {
            root.updatePolyPath()
        }

        onHeightChanged: {
            console.log(root.height)
            root.updatePolyPath()
        }
    }

    BlockConnector {
        id: output_
        rootParent: root
        width: root.width > root.minWidth ? root.width : root.minWidth
        y: 0
    }

    onHeightChanged: {
        root.updatePolyPath()
    }
}
