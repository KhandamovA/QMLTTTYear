#include "datacontext.h"

#include <QJsonArray>

BlockConstructor::BlockConstructor(
    QString group, int type, bool hasInput, bool hasOutput, QString bodyColor, QString textColor)
{
    data.group = group;

    data.type = type;
    data.blockShape = BlockData::Block;
    data.hasInput = hasInput;
    data.hasOutput = hasOutput;
    data.bodyColor = bodyColor;
    data.textColor = textColor;

    data.viewTexts.append("");
}

BlockConstructor &BlockConstructor::text(const QString &text)
{
    data.viewTexts[currentRow].append(text);
    return (*this);
}

BlockConstructor &BlockConstructor::slot(const QString &placeholder)
{
    data.viewTexts[currentRow] += " $$ ";
    data.slotsPlaceHolders.append(placeholder);
    return (*this);
}

BlockConstructor &BlockConstructor::addContainer()
{
    currentRow++;
    data.viewTexts.append("");
    return (*this);
}

BlockConstructor &BlockConstructor::comboBox(
    std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant>)> callCurrentList)
{
    data.viewTexts[currentRow] += " ?? ";
    data.comboBoxCallCurrentList.append(callCurrentList);
    return (*this);
}

BlockConstructor &BlockConstructor::button(
    std::function<QPair<QString, QVariant>(QPair<QString, QVariant>)> callSetterNewValue)
{
    data.viewTexts[currentRow] += " ** ";
    data.buttonSettersNewValue.append(callSetterNewValue);
    return (*this);
}

ReporterConstructor::ReporterConstructor(QString group,
                                         int type,
                                         QString bodyColor,
                                         QString textColor)
{
    data.type = type;
    data.blockShape = BlockData::Reporter;
    data.group = group;
    data.bodyColor = bodyColor;
    data.textColor = textColor;

    data.viewTexts.append("");
}

ReporterConstructor &ReporterConstructor::text(const QString &text)
{
    data.viewTexts[currentRow].append(text);
    return (*this);
}

ReporterConstructor &ReporterConstructor::slot(const QString &placeholder)
{
    data.viewTexts[currentRow].append(" $$ ");
    data.slotsPlaceHolders.append(placeholder);
    return (*this);
}

ReporterConstructor &ReporterConstructor::comboBox(
    std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant>)> callCurrentList)
{
    data.viewTexts[currentRow] += " ?? ";
    data.comboBoxCallCurrentList.append(callCurrentList);
    return (*this);
}

ReporterConstructor &ReporterConstructor::button(
    std::function<QPair<QString, QVariant>(QPair<QString, QVariant>)> callSetterNewValue)
{
    data.viewTexts[currentRow] += " ** ";
    data.buttonSettersNewValue.append(callSetterNewValue);
    return (*this);
}

QJsonObject BlockData::toJson() const
{
    QJsonObject obj;
    obj["type"] = type;
    obj["viewTexts"] = QJsonArray::fromStringList(viewTexts);
    obj["hasInput"] = hasInput;
    obj["hasOutput"] = hasOutput;
    obj["textColor"] = textColor;
    obj["bodyColor"] = bodyColor;
    obj["blockShape"] = blockShape;
    obj["group"] = group;
    obj["origin"] = origin;
    obj["tags"] = tags;
    obj["slotsPlaceHolders"] = QJsonArray::fromStringList(slotsPlaceHolders);
    return obj;
}

BlockData BlockData::fromJson(const QJsonObject &obj)
{
    BlockData data;
    if (obj.contains("type"))
        data.type = obj["type"].toInt();

    if (obj.contains("viewTexts")) {
        data.viewTexts.clear();
        QJsonArray arr = obj["viewTexts"].toArray();
        for (const auto &val : std::as_const(arr)) {
            data.viewTexts.append(val.toString());
        }
    }

    if (obj.contains("slotsPlaceHolders")) {
        data.slotsPlaceHolders.clear();
        QJsonArray arr = obj["slotsPlaceHolders"].toArray();
        for (const auto &val : std::as_const(arr)) {
            data.slotsPlaceHolders.append(val.toString());
        }
    }

    if (obj.contains("hasInput"))
        data.hasInput = obj["hasInput"].toBool();
    if (obj.contains("hasOutput"))
        data.hasOutput = obj["hasOutput"].toBool();
    if (obj.contains("textColor"))
        data.textColor = obj["textColor"].toString();
    if (obj.contains("bodyColor"))
        data.bodyColor = obj["bodyColor"].toString();
    if (obj.contains("blockShape"))
        data.blockShape = obj["blockShape"].toInt();
    if (obj.contains("group"))
        data.group = obj["group"].toString();
    if (obj.contains("origin"))
        data.origin = obj["origin"].toInt();
    if (obj.contains("tags")) {
        data.tags = obj["tags"].toObject();
    }
    return data;
}

DataContext::DataContext(QObject *parent)
    : QObject{parent}
{
    addStandartBlocks();
}

void DataContext::addStandartBlocks()
{
    std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant> lastValue)>
        comboBoxListVariablesNames = [this](QPair<QString, QVariant> lastValue) {
            QList<QPair<QString, QVariant>> ret;
            for (auto &i : variables.keys()) {
                QPair<QString, QVariant> v;
                v.first = i;
                v.second = i;
                ret.append(v);
            }
            return ret;
        };

    int startControlType = 0;
    // Повторять пока (while)
    systemBlocksInfo
        .insert(startControlType + 0,
                BlockConstructor("Управление", startControlType + 0, true, true, "#bfcdd9", "black")
                    .text("Повторять пока ")
                    .slot("true")
                    .addContainer());

    // Повторить N раз
    systemBlocksInfo
        .insert(startControlType + 1,
                BlockConstructor("Управление", startControlType + 1, true, true, "#bfcdd9", "black")
                    .text("Повторить ")
                    .slot("count")
                    .text(" раз")
                    .addContainer());

    // Если...то...иначе
    systemBlocksInfo
        .insert(startControlType + 2,
                BlockConstructor("Управление", startControlType + 2, true, true, "#bfcdd9", "black")
                    .text("Если ")
                    .slot("true")
                    .text(" тогда")
                    .addContainer()
                    .text(" Иначе")
                    .addContainer());

    // Если...то
    systemBlocksInfo
        .insert(startControlType + 3,
                BlockConstructor("Управление", startControlType + 3, true, true, "#bfcdd9", "black")
                    .text("Если ")
                    .slot("true")
                    .text(" тогда")
                    .addContainer());

    // Вывод в консоль (отладка)
    systemBlocksInfo
        .insert(startControlType + 4,
                BlockConstructor("Отладка", startControlType + 4, true, true, "#bfcdd9", "black")
                    .text("Вывод в консоль ")
                    .slot("text"));

    systemBlocksInfo.insert(startControlType + 5,
                            BlockConstructor("Пользовательские блоки",
                                             startControlType + 5,
                                             true,
                                             false,
                                             "#bfcdd9",
                                             "black")
                                .text("Вернуть значение в определение ")
                                .slot("any"));

    // ==================== 1. ОБЫЧНЫЕ ПЕРЕМЕННЫЕ ====================

    startControlType = 100;
    // Значение (репортер)
    systemBlocksInfo
        .insert(startControlType + 0,
                ReporterConstructor("Переменные", startControlType + 0, "#bfcdd9", "black")
                    .text("Значение ")
                    .comboBox(comboBoxListVariablesNames));

    // Установить (блок)
    systemBlocksInfo
        .insert(startControlType + 1,
                BlockConstructor("Переменные", startControlType + 1, true, true, "#bfcdd9", "black")
                    .text("Установить ")
                    .comboBox(comboBoxListVariablesNames)
                    .text(" в значение ")
                    .slot("any"));

    // ==================== 2. МАССИВЫ ====================

    // Очистить массив
    systemBlocksInfo
        .insert(startControlType + 2,
                BlockConstructor("Массивы", startControlType + 2, true, true, "#bfcdd9", "black")
                    .text("Очистить массив ")
                    .comboBox(comboBoxListVariablesNames));

    // Добавить в конец
    systemBlocksInfo
        .insert(startControlType + 3,
                BlockConstructor("Массивы", startControlType + 3, true, true, "#bfcdd9", "black")
                    .text("Добавить в массив ")
                    .comboBox(comboBoxListVariablesNames)
                    .text(" значение ")
                    .slot("any"));

    // Удалить по индексу
    systemBlocksInfo
        .insert(startControlType + 4,
                BlockConstructor("Массивы", startControlType + 4, true, true, "#bfcdd9", "black")
                    .text("Удалить из массива ")
                    .comboBox(comboBoxListVariablesNames)
                    .text(" элемент ")
                    .slot("index"));

    // Значение по индексу (репортер)
    systemBlocksInfo.insert(startControlType + 5,
                            ReporterConstructor("Массивы", startControlType + 5, "#bfcdd9", "black")
                                .text("Значение элемента ")
                                .slot("index")
                                .text(" из массива ")
                                .comboBox(comboBoxListVariablesNames));

    // Индекс по значению (позиция)
    systemBlocksInfo.insert(startControlType + 6,
                            ReporterConstructor("Массивы", startControlType + 6, "#bfcdd9", "black")
                                .text("Индекс элемента ")
                                .slot("any")
                                .text(" в массиве ")
                                .comboBox(comboBoxListVariablesNames));

    // Количество элементов (длина)
    systemBlocksInfo.insert(startControlType + 7,
                            ReporterConstructor("Массивы", startControlType + 7, "#bfcdd9", "black")
                                .text("Длина массива ")
                                .comboBox(comboBoxListVariablesNames));

    // Заменить по индексу
    systemBlocksInfo
        .insert(startControlType + 8,
                BlockConstructor("Массивы", startControlType + 8, true, true, "#bfcdd9", "black")
                    .text("Заменить в массиве ")
                    .comboBox(comboBoxListVariablesNames)
                    .text(" элемент ")
                    .slot("index")
                    .text(" на значение ")
                    .slot("any"));

    // ==================== 3. MAP (Словари/Объекты) ====================

    // Установить по ключу
    systemBlocksInfo
        .insert(startControlType + 9,
                BlockConstructor("Словари", startControlType + 9, true, true, "#bfcdd9", "black")
                    .text("Установить в словарь ")
                    .comboBox(comboBoxListVariablesNames)
                    .text(" по ключу ")
                    .slot("key")
                    .text(" значение ")
                    .slot("any"));

    // Очистить словарь
    systemBlocksInfo
        .insert(startControlType + 10,
                BlockConstructor("Словари", startControlType + 10, true, true, "#bfcdd9", "black")
                    .text("Очистить словарь ")
                    .comboBox(comboBoxListVariablesNames));

    // Количество ключей
    systemBlocksInfo.insert(startControlType + 11,
                            ReporterConstructor("Словари", startControlType + 11, "#bfcdd9", "black")
                                .text("Кол-во ключей в словаре ")
                                .comboBox(comboBoxListVariablesNames));

    // Ключ по индексу
    systemBlocksInfo.insert(startControlType + 12,
                            ReporterConstructor("Словари", startControlType + 12, "#bfcdd9", "black")
                                .text("Ключ по индексу ")
                                .slot("index")
                                .text(" из словаря ")
                                .comboBox(comboBoxListVariablesNames));

    // Значение по ключу
    systemBlocksInfo.insert(startControlType + 13,
                            ReporterConstructor("Словари", startControlType + 13, "#bfcdd9", "black")
                                .text("Значение по ключу ")
                                .slot("key")
                                .text(" из словаря ")
                                .comboBox(comboBoxListVariablesNames));

    // Удалить по ключу
    systemBlocksInfo
        .insert(startControlType + 14,
                BlockConstructor("Словари", startControlType + 14, true, true, "#bfcdd9", "black")
                    .text("Удалить из словаря ")
                    .comboBox(comboBoxListVariablesNames)
                    .text(" ключ ")
                    .slot("key"));

    // ==================== 0. ОПЕРАТОРЫ ====================

    startControlType = 200;
    // Бинарные операторы
    systemBlocksInfo.insert(startControlType + 0,
                            ReporterConstructor("Операторы", startControlType + 0, "#bfcdd9", "black")
                                .slot("any")
                                .text(" + ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 1,
                            ReporterConstructor("Операторы", startControlType + 1, "#bfcdd9", "black")
                                .slot("any")
                                .text(" - ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 2,
                            ReporterConstructor("Операторы", startControlType + 2, "#bfcdd9", "black")
                                .slot("any")
                                .text(" * ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 3,
                            ReporterConstructor("Операторы", startControlType + 3, "#bfcdd9", "black")
                                .slot("any")
                                .text(" / ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 4,
                            ReporterConstructor("Операторы", startControlType + 4, "#bfcdd9", "black")
                                .slot("any")
                                .text(" ^ ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 5,
                            ReporterConstructor("Операторы", startControlType + 5, "#bfcdd9", "black")
                                .slot("any")
                                .text(" % ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 6,
                            ReporterConstructor("Операторы", startControlType + 6, "#bfcdd9", "black")
                                .slot("string")
                                .text(" ++ ")
                                .slot("string"));

    systemBlocksInfo.insert(startControlType + 7,
                            ReporterConstructor("Операторы", startControlType + 7, "#bfcdd9", "black")
                                .slot("any")
                                .text(" == ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 8,
                            ReporterConstructor("Операторы", startControlType + 8, "#bfcdd9", "black")
                                .slot("any")
                                .text(" != ")
                                .slot("any"));

    systemBlocksInfo.insert(startControlType + 9,
                            ReporterConstructor("Операторы", startControlType + 9, "#bfcdd9", "black")
                                .slot("any")
                                .text(" > ")
                                .slot("any"));

    systemBlocksInfo
        .insert(startControlType + 10,
                ReporterConstructor("Операторы", startControlType + 10, "#bfcdd9", "black")
                    .slot("any")
                    .text(" < ")
                    .slot("any"));

    systemBlocksInfo
        .insert(startControlType + 11,
                ReporterConstructor("Операторы", startControlType + 11, "#bfcdd9", "black")
                    .slot("any")
                    .text(" >= ")
                    .slot("any"));

    systemBlocksInfo
        .insert(startControlType + 12,
                ReporterConstructor("Операторы", startControlType + 12, "#bfcdd9", "black")
                    .slot("any")
                    .text(" <= ")
                    .slot("any"));

    systemBlocksInfo
        .insert(startControlType + 13,
                ReporterConstructor("Операторы", startControlType + 13, "#bfcdd9", "black")
                    .slot("any")
                    .text(" && ")
                    .slot("any"));

    systemBlocksInfo
        .insert(startControlType + 14,
                ReporterConstructor("Операторы", startControlType + 14, "#bfcdd9", "black")
                    .slot("any")
                    .text(" || ")
                    .slot("any"));

    // Унарные операторы
    systemBlocksInfo
        .insert(startControlType + 15,
                ReporterConstructor("Операторы", startControlType + 15, "#bfcdd9", "black")
                    .text("!")
                    .slot("any"));

    systemBlocksInfo
        .insert(startControlType + 16,
                ReporterConstructor("Операторы", startControlType + 16, "#bfcdd9", "black")
                    .text("-")
                    .slot("number"));

    for (auto &i : systemBlocksInfo) {
        i.origin = 2;
    }
}
