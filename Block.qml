// pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes

Item {
    id: root
    property Item rootParent: null

    width: shape.width
    height: shape.height

    property int uid: -1
    readonly property bool isBlock: true

    property var viewTexts: ["Если $$", "Иначе", ""]
    property bool hasInput: true
    property string textColor: "black"
    property string bodyColor: "#cccccc"

    property var prevConnector: null
    property var prevContainer: null
    property var prevBlock: null
    property var nextBlock: null
    property int chainHeight: nextBlock ? root.height + nextBlock.chainHeight - props.arrowHeight : root.height - props.arrowHeight
    property var lastBlock: nextBlock ? nextBlock.lastBlock : root

    property var blockConnectors: []
    property var objectsGridPos: ({})
    property alias shape: shape

    onViewTextsChanged: {
        let texts = viewTexts
        if (texts.length > 1) {
            repeater.model = texts.slice(1)
        } else {
            repeater.model = []
        }
    }

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

        function updateBlockConnectors() {
            let buffer = []

            buffer.push({
                "rect": Utils._rectFromScene(input_),
                "connector": input_
            })

            for (let i of containers.children) {
                if ("isContainer" in i) {
                    buffer.push({
                        "rect": Utils._rectFromScene(i.output_),
                        "connector": i.output_
                    })
                }
            }

            buffer.push({
                "rect": Utils._rectFromScene(output_),
                "connector": output_
            })

            root.blockConnectors = buffer
        }
    }

    function updatePolyPath() {
        props.updatePolyPath()

        if (nextBlock) {
            nextBlock.x = 0
            nextBlock.y = root.height - props.arrowHeight
        }
    }

    function updateBlockConnectors() {
        props.updateBlockConnectors()
    }

    function setNextBlock(block) {
        let b_ = null
        if (block) {
            block.prevBlock = root
            root.nextBlock = block

            block.parent = root
            block.x = 0
            block.y = root.height - props.arrowHeight
            block.updateSceneInfo()
            b_ = block
        } else {
            if (root.nextBlock) {
                let next = root.nextBlock
                let rect = Utils._rectFromScene(next)
                next.parent = Utils.sceneContainer
                next.prevBlock = null
                next.x = rect.x
                next.y = rect.y
                root.nextBlock = null
                next.updateSceneInfo()
                b_ = next
            }
        }

        updateSceneInfo()

        if (b_)
            b_.updateSceneInfo()
    }

    function updateSceneInfo() {
        for (let i of blockConnectors) {
            let conn = i.connector
            let p = conn.rootParent
            if (p == root)
                continue
            if (p.nextBlock) {
                p.nextBlock.updateSceneInfo()
            }
        }

        updatePolyPath()
        Utils.changeGridPos(root)
        updateBlockConnectors()

        if (root.nextBlock) {
            root.nextBlock.updateSceneInfo()
        }
    }

    Component.onCompleted: {
        updateSceneInfo()
    }

    Shape {
        id: shape
        // width: root.width
        // height: root.height
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
            dragThreshold: 5

            onActiveChanged: {
                if (!active) {
                    let item = null
                    let next = null
                    let busy = false
                    let cand = Utils.candidateConnector
                    if (Utils.candidateConnector) {
                        item = cand.rootParent
                        next = item.nextBlock
                        busy = cand.busy
                    }

                    if (root.prevBlock) {
                        if (item == root.prevBlock) {
                            root.x = 0
                            root.y = item.height - props.arrowHeight
                            cand.candidate = false
                            Utils.candidateConnector = null
                            return
                        } else {
                            if (root.prevBlock)
                                root.prevBlock.setNextBlock(null)

                            root.prevConnector.busy = false
                            root.prevConnector = null
                        }
                    }

                    if (root.prevContainer) {
                        if (item == root.prevContainer) {
                            root.x = 0
                            root.y = 0
                            cand.candidate = false
                            Utils.candidateConnector = null
                            return
                        } else {
                            if (root.prevContainer)
                                root.prevContainer.setNextBlock(null)

                            root.prevConnector.busy = false
                            root.prevConnector = null
                        }
                    }

                    // Устанавливаем нового родителя
                    if (Utils.candidateConnector) {
                        let item = cand.rootParent

                        if (busy) {
                            item.setNextBlock(null)
                            item.setNextBlock(root)
                            root.lastBlock.setNextBlock(next)

                            root.prevConnector = cand
                            root.prevConnector.busy = true
                        } else {
                            item.setNextBlock(root)
                            root.prevConnector = cand
                            root.prevConnector.busy = true
                        }

                        cand.candidate = false
                        Utils.candidateConnector = null
                    }

                    if (root.x < 0)
                        root.x = 0

                    if (root.y < 0)
                        root.y = 0

                    root.updateSceneInfo()
                } else {
                    Utils.raise(root)
                }
            }

            onCentroidChanged: {
                if (dragHandler.active) {
                    let rect = Utils._rectFromScene(input_)
                    let connectors = Utils.getCandidateBlockByRect(rect, root.blockConnectors);
                    // console.log(rect, connectors)

                    let output = null
                    for (let i of connectors) {
                        if ((i.connector.isOutput && !i.connector.busy) || i.connector == root.prevConnector) {
                            output = i.connector
                            break
                        }
                    }

                    console.log(rect, output)

                    if (output == Utils.candidateConnector)
                        return
                    if (Utils.candidateConnector) {
                        Utils.candidateConnector.candidate = false
                        Utils.candidateConnector = null
                    }

                    Utils.candidateConnector = output

                    if (Utils.candidateConnector) {
                        Utils.candidateConnector.candidate = true
                        //
                    }
                }
            }
        }
    }

    BlockConnector {
        id: input_
        rootParent: root
        isOutput: false
        width: root.width
        y: 0
    }

    BlockTitle {
        id: view
        rootParent: root
        viewText: root.viewTexts.length > 0 ? root.viewTexts[0] : ""
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
            id: repeater
            model: []

            delegate: Container {
                rootParent: root
                viewText: modelData
                textColor: root.textColor

                onWidthChanged: {
                    root.updateSceneInfo()
                }
            }
        }

        onHeightChanged: {
            root.updateSceneInfo()
        }
    }

    BlockConnector {
        id: output_
        rootParent: root
        isOutput: true
        width: root.width
        y: root.height - props.arrowHeight
    }
}
