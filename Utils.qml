pragma Singleton
import QtQuick

QtObject {
    property var objectsGrid: ({})
    property int objectsGridSize: 64

    function changeGridPos(target) {
        let uid = target.uid

        // 1. УДАЛЕНИЕ СТАРОГО (используем сохраненные старые координаты)
        let oldEndX = target.objectsGridPos.oldX + target.objectsGridPos.oldW
        let oldEndY = target.objectsGridPos.oldY + target.objectsGridPos.oldH

        for (let col = target.objectsGridPos.oldX; col < oldEndX; col++) {
            if (objectsGrid[col]) {
                for (let row = target.objectsGridPos.oldY; row < oldEndY; row++) {
                    if (objectsGrid[col][row]) {
                        objectsGrid[col][row] = objectsGrid[col][row].filter(id => id !== uid)
                    }
                }
            }
        }

        // 2. ВЫЧИСЛЕНИЕ НОВЫХ КЛЕТОК
        // Переводим пиксели (x, y, width, height) в индексы сетки
        let newStartX = Math.floor(target.x / objectsGridSize)
        let newStartY = Math.floor(target.y / objectsGridSize);
        // Вычисляем сколько клеток объект занимает в ширину и высоту
        let cellsW = Math.ceil(target.width / objectsGridSize)
        let cellsH = Math.ceil(target.height / objectsGridSize)

        let newEndX = newStartX + cellsW
        let newEndY = newStartY + cellsH

        // 3. ЗАПИСЬ В НОВЫЕ КЛЕТКИ
        for (let col = newStartX; col < newEndX; col++) {
            if (objectsGrid[col] === undefined)
                objectsGrid[col] = []

            for (let row = newStartY; row < newEndY; row++) {
                if (objectsGrid[col][row] === undefined) {
                    objectsGrid[col][row] = []
                }
                // Добавляем UID, если его там еще нет
                if (objectsGrid[col][row].indexOf(uid) === -1) {
                    objectsGrid[col][row].push(uid)
                }
            }
        }

        // 4. СОХРАНЯЕМ ТЕКУЩИЕ КООРДИНАТЫ КАК СТАРЫЕ (для следующего шага)
        target.objectsGridPos.oldX = newStartX
        target.objectsGridPos.oldY = newStartY
        target.objectsGridPos.oldW = cellsW
        target.objectsGridPos.oldH = cellsH

        console.log(Object.keys(objectsGrid))
    }
}
