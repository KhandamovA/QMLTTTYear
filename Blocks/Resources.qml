pragma Singleton
import QtQuick

QtObject {
    // ============================= СОХРАНЕНИЕ начало =================================
    // Упаковка элементов заголовка
    function titleToJson(owner, title) {
        let ret = ({})

        let slots = []
        let buttons = []
        let comboBoxs = []

        let childs = Utils._findChildWithProps(title, ["isSlot", "isComboBoxSlot", "isButtonSlot"], 4)
        for (let i of childs) {
            if (i.ownerBlock !== owner)
                continue
            if ("isSlot" in i) {
                if (i.reporter) {
                    slots.push(sceneItemToJson(i.reporter))
                } else {
                    slots.push(i.inputValue)
                }
            } else if ("isComboBoxSlot" in i) {
                comboBoxs.push(i.currentValue)
            } else if ("isButtonSlot" in i) {
                buttons.push(i.currentValue)
            }
        }

        if (slots.length > 0)
            ret.slots = slots

        if (comboBoxs.length > 0)
            ret.slots = comboBoxs

        if (buttons.length > 0)
            ret.slots = buttons

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
            slots.push(titleToJson(target, i))
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
        json.debug = debug
        json.x = target.x
        json.y = target.y

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

    // Получение всех цепочек на сцене
    function sceneChainsToJson() {
        let ret = []
        let blocks = Utils._findChildWithProps(Utils.sceneContainer, ["isBlock", "isReporter"], 2)

        for (let i of blocks) {
            ret.push(chainToJson(i))
        }

        return ret
    }
    // ============================= СОХРАНЕНИЕ конец =================================

    // ============================= ЗАГРУЗКА начало =================================

    // ============================= ЗАГРУЗКА конец =================================
}
