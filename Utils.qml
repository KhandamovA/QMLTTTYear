pragma Singleton
import QtQuick

QtObject {
    property var objectsGrid: ({})
    property int objectsGridSize: 250
    property var sceneContainer: null

    property var sceneItems: ({})

    function init(sceneContainer_) {
        sceneContainer = sceneContainer_

        console.log('Инициализация успешна', sceneContainer)
    }

    function raise(target) {
        let parent = target.parent
        target.parent = null
        target.parent = parent
    }

    function registerSceneItem(target) {
        let uid = target.uid
        sceneItems[uid] = target
        changeGridPos(target)

        console.log("new item uid:", target, uid)
    }

    function changeGridPos(target) {
        let uid = target.uid

        // ВЫЧИСЛЕНИЕ НОВЫХ КЛЕТОК
        let newStartX = Math.floor(target.x / objectsGridSize)
        let newStartY = Math.floor(target.y / objectsGridSize);

        // Конечные индексы (НЕ включительно, для удобства итерации)
        let newEndX = Math.floor((target.x + target.width - 0.001) / objectsGridSize) + 1
        let newEndY = Math.floor((target.y + target.height - 0.001) / objectsGridSize) + 1

        let cellsW = newEndX - newStartX
        let cellsH = newEndY - newStartY

        // Проверка что есть изменения для обновления сетки
        if (target.objectsGridPos.oldX === newStartX && target.objectsGridPos.oldY === newStartY && target.objectsGridPos.oldW === cellsW && target.objectsGridPos.oldH === cellsH) {
            return
        }

        // console.log("NEW:", newStartX, newStartY, "w:", cellsW, "h:", cellsH);

        // 1. УДАЛЕНИЕ СТАРОГО
        let oldEndX = target.objectsGridPos.oldX + target.objectsGridPos.oldW
        let oldEndY = target.objectsGridPos.oldY + target.objectsGridPos.oldH

        // console.log("OLD:", target.objectsGridPos.oldX, target.objectsGridPos.oldY, "w:", target.objectsGridPos.oldW, "h:", target.objectsGridPos.oldH)

        for (let col = target.objectsGridPos.oldX; col < oldEndX; col++) {
            if (objectsGrid[col]) {
                for (let row = target.objectsGridPos.oldY; row < oldEndY; row++) {
                    if (objectsGrid[col][row]) {
                        const index = objectsGrid[col][row].indexOf(uid)
                        if (index !== -1) {
                            objectsGrid[col][row].splice(index, 1)
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
                    objectsGrid[col][row].push(uid)
                    // console.log("ADD:", col, row)
                }
            }
        }

        // 3. СОХРАНЯЕМ НОВЫЕ КООРДИНАТЫ
        target.objectsGridPos.oldX = newStartX
        target.objectsGridPos.oldY = newStartY
        target.objectsGridPos.oldW = cellsW
        target.objectsGridPos.oldH = cellsH
    }

    function getItemsForGrid(x, y) {
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
                console.log(container_, result.length)

                return result
            }
        }

        // 3. Если ничего не найдено, возвращаем пустой массив
        return []
    }
}
