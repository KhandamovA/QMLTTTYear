pragma Singleton
import QtQuick

QtObject {
    // ============================= СОХРАНЕНИЕ начало =================================
    // Упаковка элементов заголовка
    function titleToJson(owner, title) {
        let ret = []

        let childs = Utils._findChildWithProps(title, ["isSlot", "isComboBoxSlot", "isButtonSlot"], 4)
        for (let i of childs) {
            if (i.ownerBlock !== owner)
                continue
            if ("isSlot" in i) {
                if (i.reporter) {
                    ret.push({
                        "type": 0,
                        "data": sceneItemToJson(i.reporter)
                    })
                } else {
                    ret.push({
                        "type": 0,
                        "data": i.inputValue
                    })
                }
            } else if ("isComboBoxSlot" in i) {
                ret.push({
                    "type": 1,
                    "data": i.currentValue
                })
            } else if ("isButtonSlot" in i) {
                ret.push({
                    "type": 2,
                    "data": i.currentValue
                })
            }
        }

        return ret
    }

    // Упаковка всех слотов блока
    function slotsToJson(target) {
        let slots = []
        let titles = []
        let childs = Utils._findChildWithProp(target, "isContainer", 4)

        titles.push(target.title)
        for (let i of childs) {
            if (i.rootParent !== target)
                continue
            titles.push(i.title)
        }

        for (let i of titles) {
            titleToJson(target, i).forEach(x => {
                slots.push(x)
            })
        }

        return slots
    }

    // Упаковка всех вложенностей блока
    function containersToJson(target) {
        let containers = []
        let childs = Utils._findChildWithProp(target, "isContainer", 4)
        for (let i of childs) {
            if (i.rootParent !== target)
                continue
            containers.push(chainToJson(i.nextBlock))
        }
        return containers
    }

    // Упаковка блока либо репортера
    function sceneItemToJson(target) {
        let containers = []
        let debug = []
        let json = ({})

        if ("containers" in target) {
            json.containers = containersToJson(target)

            debug = [...target.viewTexts]
        } else {
            debug = [target.viewText]
        }

        json.type = target.type
        json.origin = target.origin
        json.tags = target.tags
        json.slots = slotsToJson(target)
        // json.debug = debug

        if ("isReporter" in target) {
            if (!target.currentSlot) {
                json.x = Math.round(target.x)
                json.y = Math.round(target.y)
            }
        }

        if ("isBlock" in target) {
            if (!target.prevBlock) {
                json.x = Math.round(target.x)
                json.y = Math.round(target.y)
            }
        }

        return json
    }

    // Упаковка цепочки блоков, начиная от firstBlock (по правильному здесь будет обрабатываться
    // только самостоятельные цепочки
    function chainToJson(firstBlock) {
        let chain = []
        let current = firstBlock
        while (current) {
            chain.push(sceneItemToJson(current))
            current = current.nextBlock
        }
        return chain
    }

    // Получение всех цепочек на сцене ГЛАВНЫЙ МЕТОД
    function sceneChainsToJson() {
        let ret = []
        let blocks = Utils._findChildWithProps(Utils.sceneContainer, ["isBlock", "isReporter"], 2)

        for (let i of blocks) {
            ret.push(chainToJson(i))
        }

        return ret
    }
    // ============================= СОХРАНЕНИЕ конец =================================

    // Очистка сцены и виртуальной сетки
    function clearScene() {
        let blocks = Utils._findChildWithProps(Utils.sceneContainer, ["isBlock", "isReporter"], 2);
        // Уничтожаем все блоки на сцене
        for (let i of blocks) {
            i.destroy()
        }
        // Обнуляем сетку положений элементов (их больше нет)
        Utils.objectsGrid = {}
        // Обнуляем ссылки на элементы (их больше нет)
        Utils.sceneItems = []
    }

    // ============================= ЗАГРУЗКА начало =================================

    // Загрузка на сцену всех самостоятельных цепочек ГЛАВНЫЙ МЕТОД
    function loadChainsToScene(arrayJson) {
        clearScene()

        for (let i of arrayJson) {
            chainFromJson(i)
        }
    }

    // Создание цепочки
    function chainFromJson(chainArray) {
        let blockData = {}
        let prevSceneItem = null
        let first = null
        for (let i = 0; i < chainArray.length; i++) {
            blockData = chainArray[i]
            let sceneItem = sceneItemFromJson(blockData)

            if (i > 0) {
                prevSceneItem.setNextBlock(sceneItem)
            } else {
                first = sceneItem
            }

            prevSceneItem = sceneItem
        }

        return first
    }

    // Создание блока
    function sceneItemFromJson(data) {
        let type = data.type
        let origin = data.origin
        let x = data.x ? data.x : 0
        let y = data.y ? data.y : 0

        let tags = data.tags

        let slots = data.slots
        let containers = data.containers

        let renderData = JSON.parse(JSON.stringify(Utils.blocksShop.getBlock(origin, type)))

        let tagsKeys = Object.keys(tags);

        // Модификация в определитель блока
        if (tagsKeys.includes("define")) {
            let text = "Определить: " + renderData.viewTexts[0].replace(/\$\$/g, "~~")
            renderData.viewTexts = [text]
            renderData.hasInput = false
            renderData.blockShape = 0
        }

        // Модификация в реплику
        if (tagsKeys.includes("replica")) {
            let slotName = tags.slotName
            renderData.viewTexts = [slotName]
            renderData.blockShape = 1
        }

        let sceneItem = Utils.addSceneItemFromData(x, y, renderData)
        sceneItem.tags = tags

        slotsFromJson(sceneItem, slots)
        containersFromJson(sceneItem, containers)

        return sceneItem
    }

    // Загрузка данных о слотах
    function slotsFromJson(target, slots) {
        let titles = []
        let childs = Utils._findChildWithProp(target, "isContainer", 4)

        titles.push(target.title)
        for (let i of childs) {
            if (i.rootParent !== target)
                continue
            titles.push(i.title)
        }

        let counter = 0
        for (let i of titles) {
            let tchilds = Utils._findChildWithProps(i, ["isSlot", "isComboBoxSlot", "isButtonSlot"], 4)
            for (let j of tchilds) {
                if (j.ownerBlock !== target)
                    continue
                if ("isSlot" in j) {
                    let data = slots[counter].data
                    if (typeof data === 'object') {
                        let reporter = sceneItemFromJson(data)
                        j.setReporter(reporter)
                    } else {
                        j.inputValue = String(data)
                    }
                    counter++
                } else if ("isComboBoxSlot" in j) {
                    let data = slots[counter].data
                    j.currentValue = data
                    j.restoreLastValue()
                    counter++
                } else if ("isButtonSlot" in j) {
                    let data = slots[counter].data
                    j.currentValue = data
                    counter++
                }
            }
        }
    }

    // Загрузка внутренностей контейнеров
    function containersFromJson(target, containers) {
        let childs = Utils._findChildWithProp(target, "isContainer", 4)
        let counter = 0
        for (let i of childs) {
            if (i.rootParent !== target)
                continue
            let chain = containers[counter]
            if (chain.length > 0) {
                let firstBlock = chainFromJson(chain)
                i.setNextBlock(firstBlock)
            }
            counter++
        }
    }

    // ============================= ЗАГРУЗКА конец =================================
}
