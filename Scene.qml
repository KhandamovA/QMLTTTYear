import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels

Flickable {
    id: scene
    anchors.fill: parent
    contentWidth: contentContainer.width * zoomScale
    contentHeight: contentContainer.height * zoomScale
    clip: true
    focus: true

    maximumFlickVelocity: 0
    // boundsBehavior: Flickable.StopAtBounds
    property real zoomScale: 1.0
    readonly property real minZoom: 0.2
    readonly property real maxZoom: 1.6
    readonly property bool isScene: true
    property alias containter: contentContainer

    // ВЫДЕЛЕНИЕ
    property var selectedItems: []
    property bool isSelecting: false
    property rect selectionRect: Qt.rect(0, 0, 0, 0)
    property alias menuModel: contextMenu.menuModel
    property point contextMenuPos: Qt.point(0, 0)

    signal selectionChanged(int uid, bool isSelected, int type)
    signal itemRemoved(Item item)
    signal itemAdded(Item item)
    signal actionTriggered(string type, int index)

    onContentXChanged: {
        if (scene.contentX < 0)
            scene.contentX = 0
    }
    onContentYChanged: {
        if (scene.contentY < 0)
            scene.contentY = 0
    }

    function addItem(item) {
        item.parent = contentContainer
    }

    // Колесико
    WheelHandler {
        id: wheelHandler
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

        grabPermissions: PointerHandler.TakeOverForbidden

        onWheel: event => {
            let mouseX = event.x
            let mouseY = event.y
            let zoomFactor = event.angleDelta.y > 0 ? 1.1 : 0.9
            let newScale = scene.zoomScale * zoomFactor

            if (newScale < scene.minZoom)
                newScale = scene.minZoom
            if (newScale > scene.maxZoom)
                newScale = scene.maxZoom

            if (newScale !== scene.zoomScale) {
                // Координаты точки под мышью в содержимом до масштабирования
                let contentXBefore = scene.contentX
                let contentYBefore = scene.contentY
                let mouseInContentX = (contentXBefore + mouseX) / scene.zoomScale
                let mouseInContentY = (contentYBefore + mouseY) / scene.zoomScale

                scene.zoomScale = newScale;

                // После изменения масштаба, смещаем так, чтобы точка под мышью осталась на месте
                scene.contentX = mouseInContentX * newScale - mouseX
                scene.contentY = mouseInContentY * newScale - mouseY
            }
            event.accepted = true
        }
    }

    // *** MouseArea для выделения ***
    MouseArea {
        id: selectionArea
        anchors.fill: parent
        propagateComposedEvents: true

        property point dragStart: Qt.point(0, 0)
        property bool ctrlPressed: false

        cursorShape: ctrlPressed ? Qt.CrossCursor : Qt.ArrowCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: e => {
            ctrlPressed = e.modifiers & Qt.ControlModifier

            if (e.button === Qt.RightButton) {

                // Откроется в позиции курсора
            } else if (ctrlPressed && e.button === Qt.LeftButton) {
                // *** ВЫКЛЮЧАЕМ FLICKABLE ***
                scene.interactive = false

                var contentPos = mapToItem(contentContainer, e.x, e.y)
                dragStart = Qt.point(contentPos.x, contentPos.y)
                scene.selectionRect.x = contentPos.x
                scene.selectionRect.y = contentPos.y
                scene.selectionRect.width = 0
                scene.selectionRect.height = 0
                scene.isSelecting = true;

                // Показываем прямоугольник
                selectionVisualizer.visible = true
                selectionVisualizer.x = contentPos.x
                selectionVisualizer.y = contentPos.y
                selectionVisualizer.width = 0
                selectionVisualizer.height = 0

                e.accepted = true
            } else {
                e.accepted = false
            }
        }

        onPositionChanged: e => {
            if (scene.isSelecting && ctrlPressed) {
                var contentPos = mapToItem(contentContainer, e.x, e.y)

                var x1 = Math.min(dragStart.x, contentPos.x)
                var y1 = Math.min(dragStart.y, contentPos.y)
                var x2 = Math.max(dragStart.x, contentPos.x)
                var y2 = Math.max(dragStart.y, contentPos.y)

                scene.selectionRect.x = x1
                scene.selectionRect.y = y1
                scene.selectionRect.width = x2 - x1
                scene.selectionRect.height = y2 - y1;

                // Обновляем визуальный прямоугольник
                selectionVisualizer.x = x1
                selectionVisualizer.y = y1
                selectionVisualizer.width = x2 - x1
                selectionVisualizer.height = y2 - y1

                e.accepted = true
            }
        }

        onReleased: e => {
            if (scene.isSelecting && ctrlPressed) {
                // *** ВКЛЮЧАЕМ FLICKABLE ОБРАТНО ***
                scene.interactive = true

                scene.isSelecting = false
                scene.selectionRect = Qt.rect(0, 0, 0, 0);

                // Прячем прямоугольник
                selectionVisualizer.visible = false

                e.accepted = true
            }
            ctrlPressed = false
        }

        onExited: {
            if (scene.isSelecting) {
                scene.interactive = true
                scene.isSelecting = false
                selectionVisualizer.visible = false
            }
            ctrlPressed = false
        }
    }

    // Контейнер
    Item {
        id: contentContainer
        width: 10000
        height: 5000
        scale: scene.zoomScale
        transformOrigin: Item.TopLeft
        property var rootParent: scene

        Rectangle {
            id: background
            anchors.fill: parent
            color: "#fbfbfb"

            Canvas {
                id: canvas
                anchors.fill: parent
                renderTarget: Canvas.Image
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.fillStyle = "#d0d0d0"
                    // цвет точек
                    var step = 36
                    var radius = 1.5
                    // радиус точки (можно чуть увеличить)

                    for (var x = 0; x <= width; x += step) {
                        for (var y = 0; y <= height; y += step) {
                            ctx.beginPath()
                            ctx.arc(x, y, radius, 0, Math.PI * 2)
                            ctx.fill()
                        }
                    }
                }
            }
        }

        // Визуализатор выделения (Rectangle)
        Rectangle {
            id: selectionVisualizer
            visible: false
            color: Qt.rgba(0, 191, 255, 0.2)
            border.color: "#00bfff"
            border.width: 2 / scene.zoomScale
            z: 10000
        }
    }

    Menu {
        id: contextMenu
        property var menuModel: []

        onOpened: {
            for (var i = 0; i < contextMenu.count; i++) {
                var item = contextMenu.itemAt(i)
                if ("text" in item) {
                    if ("condition" in menuModel[i]) {
                        item.enabled = menuModel[i].condition()
                    }
                }
            }
        }

        Repeater {
            model: contextMenu.menuModel

            delegate: DelegateChooser {
                role: "text" // Ориентируемся на поле text в твоих объектах

                // Если текст пустой — рисуем сепаратор
                DelegateChoice {
                    roleValue: ""
                    MenuSeparator {}
                }

                // Во всех остальных случаях — MenuItem
                DelegateChoice {
                    MenuItem {
                        // modelData — это твой объект {text: "...", index: ...}
                        text: modelData.text
                        enabled: true
                        onTriggered: {
                            let type = modelData.type

                            if (type == "custom") {
                                root.globalWatcher.handleActionTriggered(modelData.index, scene.contextMenuPos)
                            }

                            scene.actionTriggered(type, modelData.index)
                            // Теперь схлопнется само, но для верности:
                            contextMenu.close()
                        }
                    }
                }
            }
        }
    }
}
