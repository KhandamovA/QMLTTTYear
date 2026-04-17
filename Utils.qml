pragma Singleton
import QtQuick

QtObject {
    // Виртуальная сетка
    property var objectsGrid: ({})
    // Размер клетки у сетки
    property int objectsGridSize: 250
    // Сама сцена
    property var sceneContainer: null

    // Список со всеми элементами, uid должен быть уникальным
    property var sceneItems: ({})

    property var candidateSlot: null

    // Инициализация, обязательно указывается виджет в роли холста на котором будут распологаться все элементы
    function init(sceneContainer_) {
        sceneContainer = sceneContainer_;

        console.log('Инициализация успешна', sceneContainer);
    }

    // Поднятие элемента по z-индексу вверх
    function raise(target) {
        let parent = target.parent;
        target.parent = null;
        target.parent = parent;
    }

    // При создании нового элемента его требуется зарегистрировать на сцены для пересчета положения и оптимизации поиска
    function registerSceneItem(target) {
        let uid = target.uid;
        sceneItems[uid] = target;
        changeGridPos(target);

        console.log("new item uid:", target, uid);
    }

    // Метод перезаписи положения на виртуальной сетке сцены
    function changeGridPos(target) {
        let uid = target.uid;

        // ВЫЧИСЛЕНИЕ НОВЫХ КЛЕТОК
        let newStartX = Math.floor(target.x / objectsGridSize);
        let newStartY = Math.floor(target.y / objectsGridSize);

        // Конечные индексы (НЕ включительно, для удобства итерации)
        let newEndX = Math.floor((target.x + target.width - 0.001) / objectsGridSize) + 1;
        let newEndY = Math.floor((target.y + target.height - 0.001) / objectsGridSize) + 1;

        let cellsW = newEndX - newStartX;
        let cellsH = newEndY - newStartY;

        // Проверка что есть изменения для обновления сетки
        if (target.objectsGridPos.oldX === newStartX && target.objectsGridPos.oldY === newStartY && target.objectsGridPos.oldW === cellsW && target.objectsGridPos.oldH === cellsH) {
            return;
        }

        // console.log("NEW:", newStartX, newStartY, "w:", cellsW, "h:", cellsH);

        // 1. УДАЛЕНИЕ СТАРОГО
        let oldEndX = target.objectsGridPos.oldX + target.objectsGridPos.oldW;
        let oldEndY = target.objectsGridPos.oldY + target.objectsGridPos.oldH;

        // console.log("OLD:", target.objectsGridPos.oldX, target.objectsGridPos.oldY, "w:", target.objectsGridPos.oldW, "h:", target.objectsGridPos.oldH)

        for (let col = target.objectsGridPos.oldX; col < oldEndX; col++) {
            if (objectsGrid[col]) {
                for (let row = target.objectsGridPos.oldY; row < oldEndY; row++) {
                    if (objectsGrid[col][row]) {
                        const index = objectsGrid[col][row].indexOf(uid);
                        if (index !== -1) {
                            objectsGrid[col][row].splice(index, 1);
                            // console.log("REMOVE:", col, row)
                        }

                        if (objectsGrid[col][row].length === 0) {
                            delete objectsGrid[col][row];
                            if (Object.keys(objectsGrid[col]).length === 0) {
                                delete objectsGrid[col];
                            }
                        }
                    }
                }
            }
        }

        // 2. ЗАПИСЬ В НОВЫЕ КЛЕТКИ
        for (let col = newStartX; col < newEndX; col++) {
            if (!objectsGrid[col]) {
                objectsGrid[col] = {};
            }

            for (let row = newStartY; row < newEndY; row++) {
                if (!objectsGrid[col][row]) {
                    objectsGrid[col][row] = [];
                }

                // Проверяем, нет ли уже этого uid
                if (!objectsGrid[col][row].includes(uid)) {
                    objectsGrid[col][row].push(uid);
                    // console.log("ADD:", col, row)
                }
            }
        }

        // 3. СОХРАНЯЕМ НОВЫЕ КООРДИНАТЫ
        target.objectsGridPos.oldX = newStartX;
        target.objectsGridPos.oldY = newStartY;
        target.objectsGridPos.oldW = cellsW;
        target.objectsGridPos.oldH = cellsH;
    }

    // Получение списка элементов по позиции на сцене
    function getItemsForGridByPoint(x, y) {
        // 1. Определяем клетку по координатам
        let col = Math.floor(x / objectsGridSize);
        let row = Math.floor(y / objectsGridSize);

        // console.log(col, " ", row, " ", JSON.stringify(objectsGrid))

        // 2. Проверяем, существует ли клетка и есть ли в ней элементы
        if (col in objectsGrid) {
            if (row in objectsGrid[col]) {
                let result = [];
                let container_ = objectsGrid[col][row];
                let l = container_.length;
                for (let i = 0; i < l; i++) {
                    let item = sceneItems[container_[i]];

                    // Проверка что точка точно в фигуре
                    if (item) {
                        let pos = item.shape.mapFromItem(Utils.sceneContainer, x, y);
                        if (item.shape.contains(pos))
                            result.push(item);
                    }
                }
                // console.log(container_, result.length);

                return result;
            }
        }

        // 3. Если ничего не найдено, возвращаем пустой массив
        return [];
    }

    function getItemsForGridByRect(rect) {
        // Определяем диапазон клеток, которые пересекаются с rect
        const left = rect.x;
        const top = rect.y;
        const right = rect.x + rect.width;
        const bottom = rect.y + rect.height;

        const startCol = Math.floor(left / objectsGridSize);
        const endCol = Math.floor((right - 1e-9) / objectsGridSize);
        const startRow = Math.floor(top / objectsGridSize);
        const endRow = Math.floor((bottom - 1e-9) / objectsGridSize);

        const result = [];
        const addedIds = new Set();

        for (let col = startCol; col <= endCol; col++) {
            if (!(col in objectsGrid))
                continue;
            for (let row = startRow; row <= endRow; row++) {
                if (!(row in objectsGrid[col]))
                    continue;
                const cell = objectsGrid[col][row];
                for (let i = 0; i < cell.length; i++) {
                    const id = cell[i];
                    if (addedIds.has(id))
                        continue;

                    const item = sceneItems[id];
                    if (!item)
                        continue;

                    // Получаем прямоугольник элемента в сцене и проверяем пересечение
                    const itemRect = _rectFromScene(item);
                    if (_rectIntersection(rect, itemRect)) {
                        result.push(item);
                        addedIds.add(id);
                    }
                }
            }
        }
        return result;
    }

    // Получение слота по позиции на сцене
    function getSlotsForGrid(x, y) {
        let ret = [];
        let items = getItemsForGridByPoint(x, y);

        for (let item of items) {
            if (!item)
                continue;

            let slots = _findChildWithProp(item, "isSlot");
            for (let slot of slots) {
                let rect = _rectFromScene(slot);
                let contains = rectContains(rect, Qt.point(x, y));
                if (contains)
                    ret.push(slot);
            }
            // console.log("");

        }

        return ret;
    }

    function getSlotsForGridByRect(rect) {
        let ret = [];
        let items = getItemsForGridByRect(rect);

        for (let item of items) {
            if (!item)
                continue;

            let slots = _findChildWithProp(item, "isSlot");
            for (let slot of slots) {
                let slotRect = _rectFromScene(slot);
                if (_rectIntersection(slotRect, rect))
                    ret.push(slot);
            }
        }

        return ret;
    }

    // Получение и пометка слота в кандидаты на подключение
    function getCandidateSlot(x, y) {
        let cands = getSlotsForGrid(x, y);
        let cand = null;

        for (let i of cands) {
            if (!i.busy) {
                cand = i;
                break;
            }
        }

        if (cand == candidateSlot) {
            return candidateSlot;
        }

        if (candidateSlot) {
            candidateSlot.candidate = false;
        }

        candidateSlot = cand;

        if (candidateSlot) {
            candidateSlot.candidate = true;
        }

        return candidateSlot;
    }

    function getCandidateSlotByRect(rect, blackList = []) {
        let cands = getSlotsForGridByRect(rect);
        let cand = null;

        for (let i of cands) {
            if (!i.busy && !blackList.includes(i)) {
                cand = i;
                break;
            }
        }

        if (cand == candidateSlot) {
            return candidateSlot;
        }

        if (candidateSlot) {
            candidateSlot.candidate = false;
        }

        candidateSlot = cand;

        if (candidateSlot) {
            candidateSlot.candidate = true;
        }

        return candidateSlot;
    }

    // Мапинг геометрии в сцену
    function _rectFromScene(target) {
        let scenePos = target.mapToItem(sceneContainer, 0, 0);
        let rect = Qt.rect(scenePos.x, scenePos.y, target.width, target.height);
        return rect;
    }

    // Пересечение двух прямоугольников
    function _rectIntersection(item1, item2) {
        return (item1.x < item2.x + item2.width && item1.x + item1.width > item2.x && item1.y < item2.y + item2.height && item1.y + item1.height > item2.y);
    }

    /// Поиск элемента который содержит указанное свойство
    function _findChildWithProp(target, propName) {
        let ret = [];

        // 1. Проверка на существование объекта
        if (!target)
            return ret;

        // 2. Проверка наличия свойства (propName in target)
        if (propName in target) {
            ret.push(target);
        }

        // 3. Безопасный обход детей
        // Проверяем, есть ли дети у объекта (они есть только у наследников Item)
        if (target.children && target.children.length > 0) {
            for (let i = 0; i < target.children.length; i++) {
                let child = target.children[i];

                // Рекурсивный вызов с ОБОРУИМЯ аргументами
                let result = _findChildWithProp(child, propName);

                // Склеиваем массивы
                if (result.length > 0) {
                    ret = ret.concat(result);
                }
            }
        }

        return ret;
    }

    // Содержится ли точка в ректангле
    function rectContains(rect, point) {
        return point.x >= rect.x && point.x <= (rect.x + rect.width) && point.y >= rect.y && point.y <= (rect.y + rect.height);
    }
}
