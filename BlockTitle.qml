import QtQuick

Item {
    id: root
    property Item rootParent: null

    property string viewText: ""
    property string color: ""

    width: container.width
    height: container.height

    readonly property bool isBlockTitle: true

    property var slotsData: []

    Item {
        id: props
        property var items: []
    }

    function updateSlotsData() {
        let ret = []
        let childs = container.children
        let count = container.children.length

        for (let i = 0; i < count; i++) {
            let s = childs[i]
            if ("isSlot" in s) {
                let scene = Utils.sceneContainer
                let point = mapToItem(scene, 0, 0)

                let paddingX = point.x
                let paddingY = point.y

                let rect = Qt.rect(s.x + paddingX, s.y + paddingX, s.width, s.height)
                ret.push({
                    "item": s,
                    "rect": rect
                });

                // console.log(rect, s)
            }
        }

        slotsData = ret
    }

    onViewTextChanged: {
        let temp = []

        let buffer = ""
        let check = () => {
            if (buffer.length > 0) {
                temp.push({
                    "type": "text",
                    "value": buffer
                })
                buffer = ""
            }
        }

        for (let i = 0; i < viewText.length; i++) {
            let pair = viewText[i] + (viewText[i + 1] ? viewText[i + 1] : "")

            if (pair == "$$") {
                i += 2

                check()
                temp.push({
                    "type": "slot"
                })
            }

            if (i < viewText.length)
                buffer += viewText[i]
        }
        check()

        props.items = temp
    }

    Row {
        id: container
        spacing: 2

        Repeater {
            model: props.items
            delegate: DelegateChooser {
                role: "type"

                DelegateChoice {
                    roleValue: "text"
                    delegate: Text {
                        text: modelData.value
                        color: root.color
                    }
                }

                DelegateChoice {
                    roleValue: "slot"
                    delegate: Slot {
                        rootParent: root
                    }
                }
            }
        }
    }
}
