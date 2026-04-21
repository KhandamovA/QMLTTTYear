import QtQuick

Rectangle {
    id: root

    // --- Настройки размеров ---
    property int margins: 6
    property Item reporter: null

    // Высота фиксированная или от репортера
    height: reporter === null ? 18 : reporter.height
    // Ширина подстраивается под текст, но не меньше минимальной
    width: reporter === null ? Math.max(30, input.implicitWidth + (margins * 2)) : reporter.width

    // Идеальный радиус для капсулы — половина высоты
    radius: height / 2

    // Сглаживание включаем, но убираем слои
    antialiasing: true
    layer.enabled: false

    // Цвета из твоей логики
    color: candidate ? "blue" : "#4a4a4a" // Темно-серый вместо чисто черного (мягче)

    // Внутренняя часть (тело слота)
    Rectangle {
        anchors.fill: parent
        anchors.margins: !root.busy ? (root.candidate ? 2 : 1) : 0
        color: "white"
        radius: parent.radius - 1 // Чуть меньше, чтобы не просвечивало
        antialiasing: true
    }

    TextInput {
        id: input
        anchors.centerIn: parent // Центрируем по всему овалу
        text: "" // Для теста
        font.pixelSize: 12
        verticalAlignment: TextInput.AlignVCenter
        horizontalAlignment: TextInput.AlignHCenter
        selectByMouse: true
        color: "#333333"
        width: implicitWidth > 18 ? implicitWidth : 18
        visible: !root.busy

        // Чтобы TextInput не расширял родителя бесконечно,
        // используем implicitWidth для расчетов ширины родителя
    }

    // --- Твоя логика (без изменений) ---
    property var rootParent: null
    property bool candidate: false
    property bool busy: false
    readonly property bool isSlot: true

    function setReporter(target) {
        if (target == null) {
            if (reporter) {
                busy = false
                let scenePos = Utils._rectFromScene(reporter)
                reporter.parent = Utils.sceneContainer
                reporter.x = scenePos.x
                reporter.y = scenePos.y
                Utils.changeGridPos(reporter)
                let item = rootParent.rootParent

                if (("isReporter" in item)) {
                    Utils.changeGridPos(item)
                    reporter = null
                    return
                }

                if (("isContainer" in item)) {
                    item = item.rootParent
                    Utils.changeGridPos(item)
                    reporter = null
                    return
                }

                if (("isBlock" in item)) {
                    Utils.changeGridPos(item)
                    reporter = null
                    return
                }
            }
            return
        }

        reporter = target
        reporter.parent = root
        reporter.x = 0
        reporter.y = 0
        busy = true
    }
}
