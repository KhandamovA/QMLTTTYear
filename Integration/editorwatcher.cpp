#include "editorwatcher.h"
#include "blockeditor.h"
#include "dialogwithlist.h"

#include <QApplication>
#include <QInputDialog>
#include <QJsonValue>
#include <QMessageBox>

EditorWatcher::EditorWatcher(QObject *parent)
    : QObject{parent}
{}

void EditorWatcher::registerBlock(BlockData data, bool checkDefine)
{
    if (data.origin == 1) {
        data.type = getUniqDynamicBlockType(data.type);
        m_DynamicsBlocksInfo[data.type] = data;
    } else {
        m_blocksInfo[data.type] = data;
    }
    sendCommand("registerBlock", QJsonObject{{"data", data.toJson()}, {"checkDefine", checkDefine}});
}

QJsonObject EditorWatcher::saveScript()
{
    QJsonObject ret = sendCommand("saveScript", {}).toObject();

    // Сохранение динамически созданных блоков
    auto dynBlocksTypes = m_DynamicsBlocksInfo.keys();
    QJsonArray dynBlocksInfo;
    for (const auto &i : std::as_const(dynBlocksTypes)) {
        dynBlocksInfo.append(m_DynamicsBlocksInfo[i].toJson());
    }
    ret["dynamicBlocks"] = dynBlocksInfo;

    // Сохранение переменных
    QJsonArray variables;
    for (const auto &i : std::as_const(m_dataContext.variables)) {
        variables.append(i.name);
    }
    ret["variables"] = variables;

    return ret;
}

void EditorWatcher::clear()
{
    sendCommand("clearScript", {});
}

void EditorWatcher::loadScript(QJsonObject data)
{
    clear();

    auto dynamicBlocks = data["dynamicBlocks"].toArray();
    auto variables = data["variables"].toArray();

    data.remove("variables");
    data.remove("dynamicBlocks");

    m_DynamicsBlocksInfo.clear();

    for (const auto &i : std::as_const(dynamicBlocks)) {
        auto data = i.toObject();
        registerBlock(BlockData::fromJson(data), false);
    }
    qDebug() << "dynamicsBlocksAdded";

    m_dataContext.variables.clear();
    for (const auto &i : std::as_const(variables)) {
        auto name = i.toString();
        m_dataContext.variables[name] = "";
        m_dataContext.variables[name].name = name;
    }

    sendCommand("loadScript", data);
}

void EditorWatcher::handleResponse(QJsonValue response, qint64 signalId)
{
    emit messageGet(signalId, response);
}

QJsonValue EditorWatcher::qml_query(const QString &method, QJsonValue data)
{
    if (method == "slotPlaceholder") {
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto origin = data["origin"].toInt();

        QString text;
        if (origin == BlockData::Custom) {
            auto find = m_blocksInfo.find(type);
            if (find != m_blocksInfo.end()) {
                if (index >= 0 && index < find->slotsPlaceholders.count())
                    text = find->slotsPlaceholders[index];
            }
        } else if (origin == BlockData::Dynamic) {
            auto find = m_DynamicsBlocksInfo.find(type);
            if (find != m_DynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsPlaceholders.count())
                    text = find->slotsPlaceholders[index];
            }
        } else if (origin == BlockData::System) {
            if (type == 2 || type == 3 || type == 7) {
                return "any";
            } else if (type == 4 || type == 6) {
                return "index";
            }
        }
        return text;
    } else if (method == "comboBoxList") {
        QJsonArray ret;
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto lastKey = data["key"].toString();
        auto lastValue = data["value"].toVariant();
        auto origin = data["origin"].toInt();

        if (origin == BlockData::Custom) {
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
        } else if (origin == BlockData::Dynamic) {
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
        } else if (origin == BlockData::System) {
            // Значение переменной ??
            if ((type >= 0 && type <= 7) && index == 0) {
                return comboBoxListVariablesNames();
            }
        }

        return ret;
    } else if (method == "buttonSetter") {
        QJsonArray ret;
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto lastKey = data["key"].toString();
        auto lastValue = data["value"].toVariant();
        auto origin = data["origin"].toInt();
        QPair<QString, QVariant> newValue;

        if (origin != 1) {
            auto find = m_blocksInfo.find(type);

            if (find != m_blocksInfo.end()) {
                if (index >= 0 && index < find->buttonSettersNewValue.count()) {
                    newValue = find->buttonSettersNewValue[index]({lastKey, lastValue});
                }
            }
        } else {
            auto find = m_DynamicsBlocksInfo.find(type);

            if (find != m_DynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->buttonSettersNewValue.count()) {
                    newValue = find->buttonSettersNewValue[index]({lastKey, lastValue});
                }
            }
        }

        return QJsonObject{{"key", newValue.first}, {"value", newValue.second.toJsonValue()}};
    } else if (method == "createNewBlock") {
        createNewBlock();
    } else if (method == "createNewVar") {
        return createNewVar();
    } else if (method == "deleteVar") {
        deleteVariable();
    } else if (method == "saveToFile") {
        return saveToFile(data);
    } else if (method == "loadFromFile") {
        return loadFromFile(data.toString());
    }
    return {};
}

int EditorWatcher::getUniqDynamicBlockType(int id)
{
    while (m_DynamicsBlocksInfo.contains(id)) {
        id++;
    }
    return id;
}

QJsonValue EditorWatcher::sendCommand(const QString &method, QJsonValue data)
{
    QmlMessagePack *msg = new QmlMessagePack;
    msg->id = signalId;
    signalId++;
    connect(this, &EditorWatcher::messageGet, msg, &QmlMessagePack::messageGet);

    emit qml_signal(method, data, msg->id);

    return msg->exec();
}

void EditorWatcher::createNewBlock()
{
    BlockEditor editor(this);
    editor.exec();

    if (editor.isAccepted()) {
        auto data = editor.save();
        auto viewText = data.viewTexts[0].trimmed();
        if (!viewText.isEmpty()) {
            data.origin = 1;
            data.group = "Пользовательские блоки";
            registerBlock(data);
        }
    }
}

void EditorWatcher::deleteVariable()
{
    auto varNames = m_dataContext.variables.keys();
    if (varNames.isEmpty())
        return;

    DialogWithList dia;
    dia.addItems(varNames);
    dia.exec();
    QString selected = dia.selectedVariable();

    if (!selected.isEmpty()) {
        m_dataContext.variables.remove(selected);
    }
}

bool EditorWatcher::saveToFile(QJsonValue data)
{
    auto obj = data.toObject();
    auto path = obj["path"].toString();
    auto data2 = obj["data"];
    QByteArray data_;
    if (data2.isArray()) {
        QJsonDocument doc(data2.toArray());
        data_ = doc.toJson();
    } else if (data2.isObject()) {
        QJsonDocument doc(data2.toObject());
        data_ = doc.toJson();
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(data_);
    return true;
}

QJsonValue EditorWatcher::loadFromFile(const QString &pathToFile)
{
    QFile file(pathToFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    auto data = file.readAll();

    auto doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        return doc.array();
    } else if (doc.isObject()) {
        return doc.object();
    }
    return "";
}

QString EditorWatcher::createNewVar(const QString &oldName)
{
    QString varName = QInputDialog::getText(nullptr,
                                            "Создание переменной",
                                            "Введите имя новой переменной",
                                            QLineEdit::Normal,
                                            oldName)
                          .trimmed();

    if (varName.isEmpty())
        return "";

    if (m_dataContext.variables.contains(varName)) {
        QMessageBox::warning(nullptr,
                             "Выберите другое имя",
                             "Переменная с таким именем уже существует, выберите другое");

        return createNewVar(varName);
    }

    m_dataContext.variables[varName] = "";
    m_dataContext.variables[varName].name = varName;

    return varName;
}

QJsonArray EditorWatcher::comboBoxListVariablesNames() const
{
    QJsonArray ret;
    for (auto &i : m_dataContext.variables.keys()) {
        QJsonObject v;
        v["key"] = i;
        v["value"] = i;
        ret.append(v);
    }
    return ret;
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
    obj["slotsPlaceHolders"] = QJsonArray::fromStringList(slotsPlaceholders);
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
        data.slotsPlaceholders.clear();
        QJsonArray arr = obj["slotsPlaceHolders"].toArray();
        for (const auto &val : std::as_const(arr)) {
            data.slotsPlaceholders.append(val.toString());
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

QJsonValue QmlMessagePack::exec()
{
    while (!stop) {
        QApplication::processEvents();
    }
    disconnect();
    deleteLater();
    return returnValue;
}

void QmlMessagePack::messageGet(qint64 id, QJsonValue value)
{
    if (id == this->id) {
        returnValue = value;
        stop = true;
    }
}
