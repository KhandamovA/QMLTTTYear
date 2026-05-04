pragma Singleton
import QtQuick

QtObject {
    // Виртуальная сетка
    property var objectsGrid: ({})
    // Размер клетки у сетки
    property int objectsGridSize: 250
    // Сама сцена
    property var sceneContainer: null

    // Боковая панель с блоками
    property var blocksShop: null

    // EditorWatcher собственной персоной
    property var watcher: null

    // Список со всеми элементами, uid должен быть уникальным
    property var sceneItems: ({})

    property var candidateSlot: null
    property var candidateConnector: null
    property var hoverHelper: null
    property var zoomScale: sceneContainer ? sceneContainer.rootParent.zoomScale : 1

    // Инициализация, обязательно указывается виджет в роли холста на котором будут располагаться все элементы
    function init(sceneContainer_, blocksShop_) {
        sceneContainer = sceneContainer_
        blocksShop = blocksShop_

        console.log('Инициализация сцены прошла успешно. Scene:', sceneContainer, "BlocksShop", blocksShop)
    }

    function addSceneItemFromData(x, y, renderData) {
        // console.log(x, y, data)

        let type = renderData.type
        let shape = renderData.blockShape

        delete renderData["blockShape"]
        delete renderData["group"]
        delete renderData["slotsPlaceHolders"]

        let componentPath = ""

        if (shape === 1) {
            componentPath = "Reporter.qml"
            // Специфика для репортера
            renderData["viewText"] = renderData["viewTexts"][0]
            delete renderData["viewTexts"]
            delete renderData["hasInput"]
            delete renderData["hasOutput"]
        } else {
            componentPath = "Block.qml"
        }

        let component = Qt.createComponent(componentPath)

        if (component.status === Component.Error) {
            console.error("Ошибка загрузки:", component.errorString())
            return null
        }

        let obj = component.createObject(sceneContainer, renderData)

        obj.x = x
        obj.y = y

        registerSceneItem(obj)

        return obj
    }

    // Проверка либо создания определителя блока который пользователь создал
    function checkDefineForDynamicBlock(type) {
        let uids = Object.keys(sceneItems)
        let exists = false
        for (let i of uids) {
            let item = sceneItems[i]
            if (item.origin !== 1)
                continue
            let tags = Object.keys(item.tags)
            let itemType = item.type

            if (itemType === type && tags.includes("define")) {
                exists = true
                break
            }
        }

        if (!exists) {
            let scene = sceneContainer.rootParent

            // 1 - Динамический блок
            let blockData = JSON.parse(JSON.stringify(blocksShop.getBlock(1, type)))

            blockData.hasInput = false
            blockData.blockShape = 0
            blockData.tags = ({})
            blockData.tags["define"] = true
            let text = "Определить: " + blockData.viewTexts[0].replace(/\$\$/g, "~~")
            blockData.viewTexts[0] = text

            addSceneItemFromData(scene.contentX / scene.zoomScale + 2, scene.contentY / scene.zoomScale + 2, blockData)
        }
    }

    // Поднятие элемента по z-индексу вверх
    function raise(target) {
        let parent = target.parent
        target.parent = null
        target.parent = parent
    }

    function qmlQuery(method, data) {
        if (watcher)
            return watcher.qml_query(method, data)
        return {}
    }

    function saveToFile(path, data) {
        return qmlQuery("saveToFile", {
            "path": path,
            "data": data
        })
    }

    function loadFromFile(path) {
        return qmlQuery("loadFromFile", path)
    }

    // При создании нового элемента его требуется зарегистрировать на сцены для пересчета положения и оптимизации поиска
    function registerSceneItem(target) {
        let uid = target.uid

        if (uid == -1)
            uid = 0

        let uids = Object.keys(sceneItems)
        while (uids.includes(`${uid}`)) {
            uid += 1
        }

        target.uid = uid

        sceneItems[uid] = target

        let scene = sceneContainer.rootParent
        scene.addItem(target)

        console.log("new item uid:", uid, target)
        changeGridPos(target);

        // Обновляем положения коннекторов после попадания на сцену, чтобы значения были корректными
        if ("isBlock" in target)
            target.updateBlockConnectors()
    }

    function destroySceneItem(target) {
        if ("isBlock" in target) {
            if (target.prevBlock) {
                let prevBlock = target.prevBlock
                prevBlock.setNextBlock(null)
            }
            if (target.prevContainer) {
                let prevBlock = target.prevContainer
                prevBlock.setNextBlock(null)
            }
        }

        if ("isReporter" in target) {
            let currentSlot = target.currentSlot
            if (currentSlot) {
                currentSlot.setReporter(null)
            }
        }
        Utils.removeFromGrid(target)
        delete Utils.sceneItems[target.uid]
        target.destroy()
    }

    // Удалить с виртуальной сетки
    function removeFromGrid(target) {
        let uid = target.uid

        if (uid == -1)
            return
        let oldEndX = target.objectsGridPos.oldX + target.objectsGridPos.oldW
        let oldEndY = target.objectsGridPos.oldY + target.objectsGridPos.oldH

        // console.log("OLD:", target.objectsGridPos.oldX, target.objectsGridPos.oldY, "w:", target.objectsGridPos.oldW, "h:", target.objectsGridPos.oldH)

        for (let col = target.objectsGridPos.oldX; col < oldEndX; col++) {
            if (objectsGrid[col]) {
                for (let row = target.objectsGridPos.oldY; row < oldEndY; row++) {
                    if (objectsGrid[col][row]) {
                        const index = objectsGrid[col][row].indexOf(uid)
                        if (index !== -1) {
                            objectsGrid[col][row].splice(index, 1);
                            // console.log("REMOVE:", col, row)
                        }

                        if (objectsGrid[col][row].length === 0) {
                            delete objectsGrid[col][row]
                            if (Object.keys(objectsGrid[col]).length === 0) {
                                delete objectsGrid[col]
                            }
                        }
                    }
                }
            }
        }
    }
    // Метод перезаписи положения на виртуальной сетке сцены
    function changeGridPos(target) {
        let uid = target.uid

        if (uid == -1)
            return
        let sceneRect = _rectFromScene(target);

        // ВЫЧИСЛЕНИЕ НОВЫХ КЛЕТОК
        let newStartX = Math.floor(sceneRect.x / objectsGridSize)
        let newStartY = Math.floor(sceneRect.y / objectsGridSize);

        // Конечные индексы (НЕ включительно, для удобства итерации)
        let newEndX = Math.floor((sceneRect.x + target.width - 0.001) / objectsGridSize) + 1
        let newEndY = Math.floor((sceneRect.y + target.height - 0.001) / objectsGridSize) + 1

        let cellsW = newEndX - newStartX
        let cellsH = newEndY - newStartY

        // Проверка что есть изменения для обновления сетки
        if (target.objectsGridPos.oldX === newStartX && target.objectsGridPos.oldY === newStartY && target.objectsGridPos.oldW === cellsW && target.objectsGridPos.oldH === cellsH) {
            return
        }

        // console.log("NEW:", newStartX, newStartY, "w:", cellsW, "h:", cellsH);

        // 1. УДАЛЕНИЕ СТАРОГО
        removeFromGrid(target);

        // 2. ЗАПИСЬ В НОВЫЕ КЛЕТКИ
        for (let col = newStartX; col < newEndX; col++) {
            if (!objectsGrid[col]) {
                objectsGrid[col] = {}
            }

            for (let row = newStartY; row < newEndY; row++) {
                if (!objectsGrid[col][row]) {
                    objectsGrid[col][row] = []
                }

                // Проверяем, нет ли уже этого uid
                if (!objectsGrid[col][row].includes(uid)) {
                    objectsGrid[col][row].push(uid);
                    // console.log("ADD:", col, row)
                }
            }
        }

        // 3. СОХРАНЯЕМ НОВЫЕ КООРДИНАТЫ
        target.objectsGridPos.oldX = newStartX
        target.objectsGridPos.oldY = newStartY
        target.objectsGridPos.oldW = cellsW
        target.objectsGridPos.oldH = cellsH
        // console.log("newPos ", uid, " ", Object.keys(target.objectsGridPos), Object.values(target.objectsGridPos))
    }

    // Получение списка элементов по позиции на сцене
    function getItemsForGridByPoint(x, y) {
        // 1. Определяем клетку по координатам
        let col = Math.floor(x / objectsGridSize)
        let row = Math.floor(y / objectsGridSize);

        // console.log(col, " ", row, " ", JSON.stringify(objectsGrid))

        // 2. Проверяем, существует ли клетка и есть ли в ней элементы
        if (col in objectsGrid) {
            if (row in objectsGrid[col]) {
                let result = []
                let container_ = objectsGrid[col][row]
                let l = container_.length
                for (let i = 0; i < l; i++) {
                    let item = sceneItems[container_[i]];

                    // Проверка что точка точно в фигуре
                    if (item) {
                        let pos = item.shape.mapFromItem(Utils.sceneContainer, x, y)
                        if (item.shape.contains(pos))
                            result.push(item)
                    }
                }
                // console.log(container_, result.length);

                return result
            }
        }

        // 3. Если ничего не найдено, возвращаем пустой массив
        return []
    }

    function getItemsForGridByRect(rect) {
        // Определяем диапазон клеток, которые пересекаются с rect
        const left = rect.x
        const top = rect.y
        const right = rect.x + rect.width
        const bottom = rect.y + rect.height

        const startCol = Math.floor(left / objectsGridSize)
        const endCol = Math.floor((right - 1e-9) / objectsGridSize)
        const startRow = Math.floor(top / objectsGridSize)
        const endRow = Math.floor((bottom - 1e-9) / objectsGridSize)

        const result = []
        const addedIds = new Set()

        for (let col = startCol; col <= endCol; col++) {
            if (!(col in objectsGrid))
                continue
            for (let row = startRow; row <= endRow; row++) {
                if (!(row in objectsGrid[col]))
                    continue
                const cell = objectsGrid[col][row]
                for (let i = 0; i < cell.length; i++) {
                    const id = cell[i]
                    if (addedIds.has(id))
                        continue
                    const item = sceneItems[id]
                    if (!item)
                        continue

                    // Получаем прямоугольник элемента в сцене и проверяем пересечение
                    const itemRect = _rectFromScene(item)
                    if (_rectIntersection(rect, itemRect)) {
                        result.push(item)
                        addedIds.add(id)
                    }
                }
            }
        }
        return result
    }

    // Получение слота по позиции на сцене
    function getSlotsForGrid(x, y) {
        let ret = []
        let items = getItemsForGridByPoint(x, y)

        for (let item of items) {
            if (!item)
                continue
            let slots = _findChildWithProp(item, "isSlot")
            for (let slot of slots) {
                let rect = _rectFromScene(slot)
                let contains = rectContains(rect, Qt.point(x, y))
                if (contains)
                    ret.push(slot)
            }
            // console.log("");

        }

        return ret
    }

    function getSlotsForGridByRect(rect) {
        let ret = []
        let items = getItemsForGridByRect(rect)

        for (let item of items) {
            if (!item)
                continue
            let slots = _findChildWithProp(item, "isSlot")
            for (let slot of slots) {
                let slotRect = _rectFromScene(slot)
                if (_rectIntersection(slotRect, rect))
                    ret.push(slot)
            }
        }

        return ret
    }

    // Получение и пометка слота в кандидаты на подключение
    function getCandidateSlot(x, y) {
        let cands = getSlotsForGrid(x, y)
        let cand = null

        for (let i of cands) {
            if (!i.busy) {
                cand = i
                break
            }
        }

        if (cand == candidateSlot) {
            return candidateSlot
        }

        if (candidateSlot) {
            candidateSlot.candidate = false
        }

        candidateSlot = cand

        if (candidateSlot) {
            candidateSlot.candidate = true
        }

        return candidateSlot
    }

    function getCandidateSlotByRect(rect, blackList = []) {
        let cands = getSlotsForGridByRect(rect)
        let cand = null

        for (let i of cands) {
            if (!i.busy && !blackList.includes(i)) {
                cand = i
                break
            }
        }

        if (cand == candidateSlot) {
            return candidateSlot
        }

        if (candidateSlot) {
            candidateSlot.candidate = false
        }

        candidateSlot = cand

        if (candidateSlot) {
            candidateSlot.candidate = true
        }

        return candidateSlot
    }

    function getCandidateBlockByRect(rect, blackList = []) {
        let ret = []
        let items = getItemsForGridByRect(rect)

        for (let i of items) {
            if (!("isBlock" in i))
                continue
            let blockConnectors = i.blockConnectors
            for (let j of blockConnectors) {
                let bad = false
                for (let o of blackList) {
                    if (o.connector === j.connector) {
                        bad = true
                        break
                    }
                }

                if (!bad && _rectIntersection(j.rect, rect)) {
                    ret.push(j)
                }
            }
        }

        return ret
    }

    // Мапинг геометрии в сцену
    function _rectFromScene(target) {
        let scenePos = target.mapToItem(sceneContainer, 0, 0)
        let rect = Qt.rect(scenePos.x, scenePos.y, target.width, target.height)
        return rect
    }

    // Пересечение двух прямоугольников
    function _rectIntersection(item1, item2) {
        // console.log(item1, item2)
        return (item1.x < item2.x + item2.width && item1.x + item1.width > item2.x && item1.y < item2.y + item2.height && item1.y + item1.height > item2.y)
    }

    /// Поиск элемента который содержит указанное свойство
    function _findChildWithProp(target, propName, maxDepth = -1, currentDepth = 0) {
        return _findChildWithProps(target, [propName], maxDepth, currentDepth)
    }

    function _findChildWithProps(target, propsNames, maxDepth = -1, currentDepth = 0) {
        let ret = [];

        // 1. Проверка на существование объекта
        if (!target)
            return ret

        // 2. Проверка наличия свойства (propName in target)
        for (let i of propsNames) {
            if (i in target) {
                ret.push(target)
                break
            }
        }

        if (maxDepth > 0) {
            currentDepth++
            if (currentDepth >= maxDepth) {
                return ret
            }
        }

        // 3. Безопасный обход детей
        // Проверяем, есть ли дети у объекта (они есть только у наследников Item)
        if (target.children && target.children.length > 0) {
            for (let i = 0; i < target.children.length; i++) {
                let child = target.children[i];

                // Рекурсивный вызов с ОБОРУИМЯ аргументами
                let result = _findChildWithProps(child, propsNames, maxDepth, currentDepth);

                // Склеиваем массивы
                if (result.length > 0) {
                    ret = ret.concat(result)
                }
            }
        }

        return ret
    }

    // Содержится ли точка в ректангле
    function rectContains(rect, point) {
        return point.x >= rect.x && point.x <= (rect.x + rect.width) && point.y >= rect.y && point.y <= (rect.y + rect.height)
    }
}
