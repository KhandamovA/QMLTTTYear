import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

Item {
    id: root

    // property var model: [
    //     {
    //         "type": 0,
    //         "viewTexts": [],
    //         "hasInput": true,
    //         "hasOutput": true,
    //         "textColor": "black",
    //         "bodyColor": "#bfcdd9",
    //         "blockShape": 0 ///< 0 - обычный блок, 1 - репортер
    //     }
    // ]

    property var model: []
    property var blocksData: ({})

    onModelChanged: {
        blocksData = ({})
        let previewComponent = Qt.createComponent("BlockPreview.qml")
        let groupComponent = Qt.createComponent("BlocksGroup.qml")
        let childs = groups.children
        for (let i of childs) {
            i.destroy()
        }

        let actualGroups = ({})
        let counter = 0
        for (let i of model) {
            counter++
            let groupName = i["group"]
            let bodyColor = i["bodyColor"]
            let keys = Object.keys(actualGroups)
            let currGroup = null
            let type = i["type"]

            blocksData[type] = i

            if (keys.includes(groupName)) {
                currGroup = actualGroups[groupName]
            } else {
                let group = groupComponent.createObject(groups, {
                    "name": groupName,
                    "bodyColor": bodyColor
                })

                actualGroups[groupName] = group
                currGroup = group
            }

            let column = currGroup.column
            let obj = previewComponent.createObject(column, {
                "blockData": i
            })
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        ScrollView {
            id: scrollView
            width: parent.width
            height: parent.height
            clip: true

            Column {
                id: groups
                spacing: 12
            }
        }
    }
}
