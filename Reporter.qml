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

    property var slots: []
    property var currentSlot: null
    property var objectsGridPos: ({})
    property alias shape: shape

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

        layer.enabled: true
        layer.samples: 8
        layer.smooth: true

        ShapePath {
            strokeWidth: 1
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
                    Utils.changeGridPos(root)

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

                    if (root.x < 0)
                        root.x = 0

                    if (root.y < 0)
                        root.y = 0
                } else {
                    Utils.raise(root)
                }
            }

            onCentroidChanged: {
                if (dragHandler.active) {
                    console.log("reporter", root.x, root.y)
                    let rect = Utils._rectFromScene(root)
                    Utils.getCandidateSlotByRect(rect, root.slots)

                    console.log(rect)
                }
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
