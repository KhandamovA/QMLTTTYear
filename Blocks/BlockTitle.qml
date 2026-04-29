import QtQuick
import QtQuick.Window

// $$ - обычный слот со значением
// ?? - выпадающий список

Item {
    id: root
    property Item rootParent: null
    property Item ownerBlock: null

    property string viewText: ""
    property string color: ""

    width: container.width
    height: container.height

    readonly property bool isBlockTitle: true
    property alias container: container

    property var slotsData: []

    Item {
        id: props
        property var items: []
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

        let slotCounter = 0
        let comboBoxCounter = 0
        let buttonCounter = 0
        for (let i = 0; i < viewText.length; i++) {
            let pair = viewText[i] + (viewText[i + 1] ? viewText[i + 1] : "")

            if (pair == "$$") {
                i += 2

                check()
                temp.push({
                    "index": slotCounter,
                    "type": "slot",
                    "placeholder": Utils.qmlQuery("slotPlaceholder", {
                        "type": root.ownerBlock.type,
                        "index": slotCounter
                    })
                })
                slotCounter++
            } else if (pair == "??") {
                i += 2

                check()
                temp.push({
                    "index": comboBoxCounter,
                    "type": "comboBoxSlot"
                })
                comboBoxCounter++
            } else if (pair == "**") {
                i += 2

                check()
                temp.push({
                    "index": buttonCounter,
                    "type": "buttonSlot"
                })
                buttonCounter++
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
                        anchors.verticalCenter: container.verticalCenter
                    }
                }

                DelegateChoice {
                    roleValue: "slot"
                    delegate: Slot {
                        rootParent: root
                        anchors.verticalCenter: container.verticalCenter
                        placeholderText: typeof modelData.placeholder === "string" ? modelData.placeholder : ""
                    }
                }

                DelegateChoice {
                    roleValue: "comboBoxSlot"
                    delegate: ComboBoxSlot {
                        rootParent: root
                        anchors.verticalCenter: container.verticalCenter
                        updateItemMethod: function () {
                            return Utils.qmlQuery("comboBoxList", {
                                "type": root.ownerBlock.type,
                                "index": modelData.index,
                                "key": selectedValue.key,
                                "value": selectedValue.value
                            })
                        }
                    }
                }

                DelegateChoice {
                    roleValue: "buttonSlot"
                    delegate: ButtonSlot {
                        rootParent: root
                        ownerBlock: root.ownerBlock
                        index: modelData.index
                        anchors.verticalCenter: container.verticalCenter
                    }
                }
            }
        }

        onHeightChanged: {
            if ("updatePolyPath" in root.rootParent) {
                root.rootParent.updatePolyPath()
            } else {
                // console.log(root.rootParent)
            }
        }
    }
}
