#include "editorwatcher.h"
#include "blockeditor.h"

#include <QApplication>
#include <QJsonValue>

EditorWatcher::EditorWatcher(QObject *parent)
    : QObject{parent}
{}

void EditorWatcher::registerBlock(BlockData data)
{
    if (data.isDynamicBlock) {
        data.type = getUniqDynamicBlockType(data.type);
        m_DynamicsBlocksInfo[data.type] = data;
    } else {
        m_blocksInfo[data.type] = data;
    }
    sendCommand("registerBlock", data.toJson());
}

void EditorWatcher::handleResponse(QJsonValue response)
{
    m_responses.append(response);
}

QJsonValue EditorWatcher::qml_query(const QString &method, QJsonValue data)
{
    if (method == "slotPlaceholder") {
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto isDynamicBlock = data["isDynamicBlock"].toBool();

        QString text;
        if (!isDynamicBlock) {
            auto find = m_blocksInfo.find(type);
            if (find != m_blocksInfo.end()) {
                if (index >= 0 && index < find->slotsPlaceholders.count())
                    text = find->slotsPlaceholders[index];
            }
        } else {
            auto find = m_DynamicsBlocksInfo.find(type);
            if (find != m_DynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsPlaceholders.count())
                    text = find->slotsPlaceholders[index];
            }
        }
        return text;
    } else if (method == "comboBoxList") {
        QJsonArray ret;
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto lastKey = data["key"].toString();
        auto lastValue = data["value"].toVariant();
        auto isDynamicBlock = data["isDynamicBlock"].toBool();

        if (!isDynamicBlock) {
            auto find = m_blocksInfo.find(type);

            if (find != m_blocksInfo.end()) {
                if (index >= 0 && index < find->comboBoxCallCurrentList.count()) {
                    auto list = find->comboBoxCallCurrentList[index]({lastKey, lastValue});

                    for (auto &i : list) {
                        QJsonObject entry;
                        entry["key"] = i.first;
                        entry["value"] = i.second.toJsonValue();
                        ret.append(entry);
                    }
                }
            }
        } else {
            auto find = m_DynamicsBlocksInfo.find(type);

            if (find != m_DynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->comboBoxCallCurrentList.count()) {
                    auto list = find->comboBoxCallCurrentList[index]({lastKey, lastValue});

                    for (auto &i : list) {
                        QJsonObject entry;
                        entry["key"] = i.first;
                        entry["value"] = i.second.toJsonValue();
                        ret.append(entry);
                    }
                }
            }
        }

        return ret;
    } else if (method == "buttonSetter") {
        QJsonArray ret;
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto lastKey = data["key"].toString();
        auto lastValue = data["value"].toVariant();

        auto newValue = m_blocksInfo[type].buttonSettersNewValue[index]({lastKey, lastValue});

        return QJsonObject{{"key", newValue.first}, {"value", newValue.second.toJsonValue()}};
    } else if (method == "createNewBlock") {
        BlockEditor editor(this);
        editor.exec();

        if (editor.isAccepted()) {
            auto data = editor.save();
            data.isDynamicBlock = true;
            data.group = "Пользовательские блоки";
            registerBlock(data);
        }
    }
    return {};
}

int EditorWatcher::getUniqDynamicBlockType(int id)
{
    while (m_DynamicsBlocksInfo.contains(id) || m_blocksInfo.contains(id)) {
        id++;
    }
    return id;
}

QJsonValue EditorWatcher::sendCommand(const QString &method, QJsonValue data)
{
    emit qml_signal(method, data);

    while (m_responses.isEmpty()) {
        QApplication::processEvents();
    }

    auto response = m_responses[0];
    m_responses.clear();
    return response;
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
    obj["isDynamicBlock"] = isDynamicBlock;
    obj["tags"] = tags;
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
    if (obj.contains("isDynamicBlock"))
        data.isDynamicBlock = obj["isDynamicBlock"].toBool();
    if (obj.contains("tags")) {
        data.tags = obj["tags"].toObject();
    }
    return data;
}

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
    data.slotsPlaceholders.append(placeholder);
    return (*this);
}

BlockConstructor &BlockConstructor::addContainer()
{
    currentRow++;
    data.viewTexts.append("");
    return (*this);
}

BlockConstructor &BlockConstructor::comboBox(
    std::function<QList<QPair<QString, QVariant> >(QPair<QString, QVariant>)> callCurrentList)
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
    data.slotsPlaceholders.append(placeholder);
    return (*this);
}

ReporterConstructor &ReporterConstructor::comboBox(
    std::function<QList<QPair<QString, QVariant> >(QPair<QString, QVariant>)> callCurrentList)
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
