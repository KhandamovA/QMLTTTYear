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
                        root.x = 0;

                    if (root.y < 0)
                        root.y = 0;

                    Utils.changeGridPos(root);
                    view.updateSlotsData();

                    let rect = Utils._rectFromScene(root);
                    let slot = Utils.getCandidateSlotByRect(rect, root.slots);

                    if (slot !== null) {
                        slot.setReporter(root);
                        Utils.candidateSlot.candidate = false;
                        Utils.candidateSlot = null;
                    }
                } else {
                    Utils.raise(root);
                }
            }

            onCentroidChanged: {
                // console.log(root.x, root.y)
                let rect = Utils._rectFromScene(root);
                Utils.getCandidateSlotByRect(rect, root.slots);

                // console.log(slot);
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
            let buffer = [];
            let childs = view.container.children;
            for (let i of childs) {
                let slots = Utils._findChildWithProp(i, "isSlot");
                for (let j of slots) {
                    buffer.push(j);
                }
            }

            root.slots = buffer;
        }
    }
}
