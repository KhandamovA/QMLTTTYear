import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels

Flickable {
    id: scene
    // anchors.fill: parent
    contentWidth: contentContainer.width * zoomScale
    contentHeight: contentContainer.height * zoomScale
    clip: true
    focus: true

    maximumFlickVelocity: 0
    // boundsBehavior: Flickable.StopAtBounds
    property real zoomScale: 1.0
    readonly property real minZoom: 0.2
    readonly property real maxZoom: 4.0
    readonly property bool isScene: true
    property alias containter: contentContainer

    // ВЫДЕЛЕНИЕ
    property var selectedItems: []
    property bool isSelecting: false
    property rect selectionRect: Qt.rect(0, 0, 0, 0)
    property alias menuModel: contextMenu.menuModel
    property point contextMenuPos: Qt.point(0, 0)
    property var targetBlock: null

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

        function wheelEvent(delta, mx, my) {
            let oldScale = scene.zoomScale
            let zoomFactor = delta > 0 ? 1.1 : 0.9
            let newScale = Math.max(scene.minZoom, Math.min(scene.maxZoom, oldScale * zoomFactor))

            if ((oldScale < 1.0 && newScale > 1.0) || (oldScale > 1.0 && newScale < 1.0))
                newScale = 1.0

            if (newScale !== oldScale) {
                // 1. Вычисляем ГДЕ мышь стоит относительно окна (viewport)
                // Раз event.x растет вместе с контентом, то разница между ним
                // и сдвигом контента (contentX) — это стабильная точка на экране.
                let stableMouseX = mx - scene.contentX
                let stableMouseY = my - scene.contentY

                // 2. Находим "чистую" координату точки в контенте (без учета масштаба)
                let contentPointX = mx / oldScale
                let contentPointY = my / oldScale

                // 3. Применяем зум
                scene.zoomScale = newScale;

                // 4. Двигаем контент
                // Новое смещение = (Точка в контенте * новый масштаб) - стабильная точка на экране
                scene.contentX = contentPointX * newScale - stableMouseX
                scene.contentY = contentPointY * newScale - stableMouseY

                zoomIndicator.show()
            }
        }

        onWheel: event => {
            wheelEvent(event.angleDelta.y, event.x, event.y)
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
            focus = true
            ctrlPressed = e.modifiers & Qt.ControlModifier

            if (e.button === Qt.RightButton) {
                // Откроется в позиции курсора
                let contentPos = mapToItem(contentContainer, e.x, e.y)
                let items = Utils.getItemsForGridByPoint(contentPos.x, contentPos.y)
                let item = null

                for (let i of items) {
                    if ("isBlock" in i || "isReporter" in i) {
                        item = i
                        break
                    }
                }

                if (item) {
                    scene.targetBlock = item
                    contextMenu.popup()
                }
            } else if (ctrlPressed && e.button === Qt.LeftButton) {
                // *** ВЫКЛЮЧАЕМ FLICKABLE ***
                scene.interactive = false

                let contentPos = mapToItem(contentContainer, e.x, e.y)
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

        DropArea {
            id: sceneDropArea
            anchors.fill: contentContainer

            // Тот самый ключ, который мы указали в магазине
            keys: ["block"]

            // Когда блок отпустили над сценой
            onDropped: drop => {
                // 1. Получаем данные, которые мы упаковали в магазине
                let data = JSON.parse(JSON.stringify(drop.source.Drag.mimeData.blockData));

                // 2. Координаты сброса (относительно DropArea)

                // Получаем размеры блока из источника
                let blockW = drop.source.width
                let blockH = drop.source.height

                // Получаем hotSpot (смещение курсора относительно угла картинки)
                let hsX = drop.source.Drag.hotSpot.x
                let hsY = drop.source.Drag.hotSpot.y;

                // Вычисляем левый верхний угол
                let finalX = drop.x - hsX
                let finalY = drop.y - hsY

                // console.log("Приняли блок:", data.type, "в координаты:", xPos, yPos);

                // 3. Вызываем твою логику создания реального блока
                let item = Utils.addSceneItemFromData(finalX, finalY, data);

                // После попадания на сцену принудительно просим чекнуть нет ли там чего к чему можно присосаться
                if ("isBlock" in item) {
                    item.checkCandidateBlock()
                    item.applyCandidateBlock()
                }

                if ("isReporter" in item) {
                    item.checkCandidateSlot()
                    item.applyCandidateSlot()
                }

                // 4. Подтверждаем системе, что дроп принят успешно
                drop.accept()
            }

            // Опционально: визуальная реакция, когда блок "занесли" над сценой
            onEntered: drag => {}

            onExited: {}
        }
    }

    Text {
        id: zoomIndicator
        // Это заставит текст игнорировать прокрутку контента
        parent: scene

        // Центрируем вручную, так как anchors иногда капризничают в Flickable
        x: (scene.width - width) / 2
        y: (scene.height - height) / 2

        z: 1000 // Гарантируем, что текст выше всех элементов сцены

        text: Math.round(scene.zoomScale * 100) + "%"
        color: "white"
        font.pixelSize: 60 // Сделаем побольше, чтобы точно заметить
        font.bold: true

        // Обводка, чтобы не сливалось
        style: Text.Outline
        styleColor: "black"

        opacity: 0
        visible: opacity > 0 // Экономим ресурсы, когда не видно

        NumberAnimation on opacity {
            id: fadeAnimation
            from: 1.0
            to: 0.0
            duration: 1000
            easing.type: Easing.OutCubic
        }

        function show() {
            fadeAnimation.stop()
            zoomIndicator.opacity = 1.0
            fadeAnimation.start()
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

                            scene.actionTriggered(type, modelData.index)
                            // Теперь схлопнется само, но для верности:
                            contextMenu.close()
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        contextMenu.menuModel = []
        let actions = [
            {
                "text": "Удалить",
                "index": 0,
                "type": "system"
            },
            {
                "text": "Дублировать",
                "index": 1,
                "type": "system",
                "condition": function () {
                    let keys = Object.keys(scene.targetBlock.tags)
                    if (keys.includes("define")) {
                        return false
                    }
                    return true
                }
            }
        ]
        contextMenu.menuModel = actions
    }

    onActionTriggered: (type, index) => {
        if (index === 0) {
            removeSelectedDialog.open()
        } else if (index === 1) {
            dublicateBlocks(targetBlock)
        }
    }

    function dublicateBlocks(target) {
        let chain = Resources.chainToJson(target)
        let rect = Utils._rectFromScene(target)
        let firstItem = Resources.chainFromJson(chain)
        firstItem.x = rect.x + 25
        firstItem.y = rect.y + 25
    }

    Dialog {
        id: removeSelectedDialog
        title: "Вы действительно хотите удалить цепочку блоков?"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent

        Component.onCompleted: {
            standardButton(Dialog.Yes).clicked.connect(() => {
                if (scene.targetBlock.origin === 1) {
                    let tagsKeys = Object.keys(scene.targetBlock.tags)
                    if (tagsKeys.includes("define")) {
                        //Удаляем отовсюду в том числе и в блоках
                        Utils.qmlQuery("removeDynamicBlock", scene.targetBlock.type)
                        Utils.blocksShop.deleteDynamicBlock(scene.targetBlock.type)
                        let uids = Object.keys(Utils.sceneItems)
                        for (let i of uids) {
                            let item = Utils.sceneItems[i]
                            if (item.origin === 1 && item.type === scene.targetBlock.type) {
                                Utils.destroySceneItem(item)
                            }
                        }
                    }
                }
                Utils.destroySceneItem(scene.targetBlock)
            })
        }
    }
}
