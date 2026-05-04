import QtQuick
import QtQuick.Shapes

Item {
    id: root
    property Item rootParent: null

    property int uid: -1

    // Этот тип указывается если блок зарегистрирован
    property int type: 0
    // Этот тип указывается если блок динамически созданный
    property int origin: 0

    property string viewText: "Значение переменной $$ $$"
    property string textColor: "black"
    property string bodyColor: "#bfcdd9"

    width: shape.width
    height: shape.height

    // Вспомогательные свойства
    property var tags: ({})
    property var slots: []
    property var currentSlot: null
    property var objectsGridPos: ({})
    property alias shape: shape
    property alias shapePath: shapePath
    property alias title: view

    readonly property bool isReporter: true

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
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            id: shapePath
            strokeWidth: 1 / Utils.sceneContainer.rootParent.zoomScale
            strokeColor: "#888888"
            fillColor: root.bodyColor
            capStyle: ShapePath.RoundCap

            PathRectangle {
                x: 1
                y: 1
                width: shape.width - 2
                height: shape.height - 2
                radius: width
                strokeAdjustment: 1
            }
        }

        HoverHandler {

            onHoveredChanged: {
                let scene = Utils.sceneContainer.rootParent
                // При масштабировании может DragHandler не работать, для обхода проблемы у сцены отключаем интерактивность
                if (hovered) {
                    Utils.hoverHelper = root
                    scene.interactive = false
                } else {
                    if (Utils.hoverHelper === root)
                        scene.interactive = true
                }
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
                    root.applyCandidateSlot()

                    if (root.x < 0)
                        root.x = 0

                    if (root.y < 0)
                        root.y = 0

                    Utils.changeGridPos(root)
                } else {
                    Utils.raise(root)
                }
            }

            onCentroidChanged: {
                if (dragHandler.active) {
                    // console.log("reporter", root.x, root.y)

                    root.checkCandidateSlot();

                    // console.log(rect)
                }
            }
        }
    }

    function checkCandidateSlot() {
        let rect = Utils._rectFromScene(root)
        Utils.getCandidateSlotByRect(rect, root.slots)
    }

    function applyCandidateSlot() {
        let rect = Utils._rectFromScene(root)
        let slot = Utils.getCandidateSlotByRect(rect, root.slots)
        let allSlots = Utils.getSlotsForGridByRect(rect)

        if (root.currentSlot && allSlots.includes(root.currentSlot)) {
            root.x = 0
            root.y = 0
        }

        if (slot !== null) {
            if (root.currentSlot) {
                root.currentSlot.setReporter(null)
                root.currentSlot = null
            }

            slot.setReporter(root)
            Utils.candidateSlot.candidate = false
            Utils.candidateSlot = null
            root.currentSlot = slot
        } else {
            if (root.currentSlot) {
                root.currentSlot.setReporter(null)
                root.currentSlot = null
            }
        }
    }

    BlockTitle {
        id: view
        rootParent: root
        ownerBlock: root
        viewText: root.viewText
        x: props.margins
        y: props.margins
        color: root.textColor

        onWidthChanged: {
            let buffer = []
            let childs = view.container.children
            for (let i of childs) {
                let slots = Utils._findChildWithProp(i, "isSlot")
                for (let j of slots) {
                    buffer.push(j)
                }
            }

            root.slots = buffer
        }
    }
}
