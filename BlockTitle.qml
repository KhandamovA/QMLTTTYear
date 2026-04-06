import QtQuick

Item {
    id: root

    property string viewText: ""
    property string color: ""

    width: container.width
    height: container.height

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
                    delegate: Slot {}
                }
            }
        }
    }
}
