import QtQuick
import QtQuick.Window

// $$ - обычный слот со значением
// ?? - выпадающий список

Item {
    id: root
    property var rootParent: null
    property var ownerBlock: null

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

        let counter = 0

        for (let i = 0; i < viewText.length; i++) {
            let pair = viewText[i] + (viewText[i + 1] ? viewText[i + 1] : "")

            if (pair == "$$") {
                i += 2

                check()
                temp.push({
                    "index": counter,
                    "type": "slot",
                    "placeholder": Utils.qmlQuery("slotPlaceholder", {
                        "type": root.ownerBlock.type,
                        "index": counter,
                        "origin": root.ownerBlock.origin
                    })
                })
            } else if (pair == "??") {
                i += 2

                check()
                temp.push({
                    "index": counter,
                    "type": "comboBoxSlot"
                })
            } else if (pair == "**") {
                i += 2

                check()
                temp.push({
                    "index": counter,
                    "type": "buttonSlot"
                })
            } else if (pair == "~~") {
                i += 2

                check()
                temp.push({
                    "index": counter,
                    "type": "replicaSlot",
                    "placeholder": Utils.qmlQuery("slotPlaceholder", {
                        "type": root.ownerBlock.type,
                        "index": counter,
                        "origin": root.ownerBlock.origin
                    })
                })
            }

            if (i < viewText.length)
                buffer += viewText[i]

            counter++
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
                        ownerBlock: root.ownerBlock
                        anchors.verticalCenter: container.verticalCenter
                        placeholderText: typeof modelData.placeholder === "string" ? modelData.placeholder : ""
                    }
                }

                DelegateChoice {
                    roleValue: "comboBoxSlot"
                    delegate: ComboBoxSlot {
                        rootParent: root
                        ownerBlock: root.ownerBlock
                        anchors.verticalCenter: container.verticalCenter
                        updateItemMethod: function () {
                            return Utils.qmlQuery("comboBoxList", {
                                "type": root.ownerBlock.type,
                                "index": modelData.index,
                                "key": currentValue.key,
                                "value": currentValue.value,
                                "origin": root.ownerBlock.origin
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

                DelegateChoice {
                    roleValue: "replicaSlot"
                    delegate: ReplicaSlot {
                        rootParent: root
                        ownerBlock: root.ownerBlock
                        index: modelData.index
                        anchors.verticalCenter: container.verticalCenter
                        blockData: {
                            // Создаем реплику
                            let slotName = modelData.placeholder
                            let replica = {
                                "type": root.ownerBlock.type,
                                "tags": {
                                    "replica": true,
                                    "slotName": slotName
                                },
                                "blockShape": 1,
                                "viewTexts": [slotName],
                                "bodyColor": root.ownerBlock.bodyColor,
                                "textColor": root.ownerBlock.textColor,
                                "origin": root.ownerBlock.origin
                            }
                            return replica
                        }
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
