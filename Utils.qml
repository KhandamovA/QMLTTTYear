pragma Singleton
import QtQuick

QtObject {
    property var objectsGrid: ({})
    property int objectsGridSize: 100
    property var sceneContainer: null

    property var sceneItems: ({})

    function init(sceneContainer_) {
        sceneContainer = sceneContainer_

        console.log('Инициализация успешна', sceneContainer)
    }

    function registerSceneItem(target){
        let uid = target.uid
        sceneItems[uid] = target
        changeGridPos(target)

        console.log("new item uid:", target, uid)
    }

    function changeGridPos(target) {
        let uid = target.uid

        // 2. ВЫЧИСЛЕНИЕ НОВЫХ КЛЕТОК
        // Переводим пиксели (x, y, width, height) в индексы сетки
        let newStartX = Math.floor(target.x / objectsGridSize)
        let newStartY = Math.floor(target.y / objectsGridSize);
        // Вычисляем сколько клеток объект занимает в ширину и высоту
        let cellsW = Math.ceil(target.width / objectsGridSize)
        let cellsH = Math.ceil(target.height / objectsGridSize)

        let newEndX = newStartX + cellsW
        let newEndY = newStartY + cellsH

        // Проверка что есть изменения для обновления сетки
        if(target.objectsGridPos.oldX === newStartX &&
                target.objectsGridPos.oldY === newStartY &&
                target.objectsGridPos.oldW == cellsW
                && target.objectsGridPos.oldH == cellsH){
            return;
        }

        // 1. УДАЛЕНИЕ СТАРОГО (используем сохраненные старые координаты)
        let oldEndX = target.objectsGridPos.oldX + target.objectsGridPos.oldW
        let oldEndY = target.objectsGridPos.oldY + target.objectsGridPos.oldH

        for (let col = target.objectsGridPos.oldX; col < oldEndX; col++) {
            if (objectsGrid[col]) {
                for (let row = target.objectsGridPos.oldY; row < oldEndY; row++) {
                    if (objectsGrid[col][row]) {
                        objectsGrid[col][row] = objectsGrid[col][row].filter(id => id !== uid)
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



        // 3. ЗАПИСЬ В НОВЫЕ КЛЕТКИ
        for (let col = newStartX; col < newEndX; col++) {
            if (objectsGrid[col] === undefined)
                objectsGrid[col] = ({})

            for (let row = newStartY; row < newEndY; row++) {
                if (objectsGrid[col][row] === undefined || objectsGrid[col][row] === null) {
                    objectsGrid[col][row] = []
                }

                // Добавляем UID, если его там еще нет
                let alreadyExists = objectsGrid[col][row].some(item => item.uid === uid); // или item.id, смотря как названо свойство

                if(!alreadyExists)
                {
                    objectsGrid[col][row].push(uid)
                }


            }
        }

        // 4. СОХРАНЯЕМ ТЕКУЩИЕ КООРДИНАТЫ КАК СТАРЫЕ (для следующего шага)
        target.objectsGridPos.oldX = newStartX
        target.objectsGridPos.oldY = newStartY
        target.objectsGridPos.oldW = cellsW
        target.objectsGridPos.oldH = cellsH

        // Object.keys(objectsGrid).forEach(col => {
        //                                      Object.keys(objectsGrid[col]).forEach(row => {
        //                                                                                let items = objectsGrid[col][row]
        //                                                                                if (items && items.length > 0) {
        //                                                                                    console.log(`Колонка [${col}], Строка [${row}]: [${items.join(", ")}]`)
        //                                                                                }
        //                                                                            })
        //                                  })

        // console.log(JSON.stringify(objectsGrid))
        console.log("")
    }

    function getItemsForGrid(x, y) {
        // 1. Определяем клетку по координатам
        let col = Math.floor(x / objectsGridSize)
        let row = Math.floor(y / objectsGridSize);

        // console.log(col, " ", row, " ", JSON.stringify(objectsGrid))


        // 2. Проверяем, существует ли клетка и есть ли в ней элементы
        if(col in objectsGrid){
            if( row in objectsGrid[col]){
                let result = []
                for(let uid in objectsGrid[col][row]){
                    let item = sceneItems[uid]

                    // Проверка что точка точно в фигуре
                    if(item){
                        let pos = item.shape.mapFromItem(Utils.sceneContainer, x, y)
                        if(item.shape.contains(pos))
                            result.push(item)
                    }

                }
                console.log(result.length)

                return result
            }
        }

        // 3. Если ничего не найдено, возвращаем пустой массив
        return []
    }
}
