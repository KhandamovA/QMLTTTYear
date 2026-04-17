import QtQuick

Rectangle {
    id: root
    property Item rootParent: null
    width: root.reporter === null ? Math.max(26, input.width + (root.margins * 2)) : root.reporter.width
    height: root.reporter === null ? 18 : root.reporter.height
    radius: root.width / 2
    antialiasing: true

    border.color: root.candidate ? "blue" : "black"
    border.width: root.candidate ? 2 : 0
    color: "white"

    property int margins: 4
    property bool candidate: false // При наведении показывает является ли он кандидатом на подключение
    property bool busy: false
    property Item reporter: null
    readonly property bool isSlot: true

    function setReporter(target) {
        if (target == null) {
            busy = false;
            reporter.parent = Utils.sceneContainer;
            let scenePos = reporter.mapToItem(Utils.sceneContainer);
            reporter.x = scenePos.x;
            reporter.y = scenePos.y;
            reporter = null;
            return;
        }
        reporter = target;
        reporter.parent = root;
        reporter.x = 0;
        reporter.y = 0;
        busy = true;
    }

    TextInput {
        id: input
        x: root.margins
        height: root.height
        text: ""
        width: implicitWidth > 18 ? implicitWidth : 18
    }
}
