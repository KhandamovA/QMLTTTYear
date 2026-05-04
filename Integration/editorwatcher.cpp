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
        m_dataContext.dynamicsBlocksInfo[data.type] = data;
    } else {
        m_dataContext.blocksInfo[data.type] = data;
    }
    sendCommand("registerBlock", QJsonObject{{"data", data.toJson()}, {"checkDefine", checkDefine}});
}

QJsonObject EditorWatcher::saveScript()
{
    QJsonObject ret = sendCommand("saveScript", {}).toObject();

    // Сохранение динамически созданных блоков
    auto dynBlocksTypes = m_dataContext.dynamicsBlocksInfo.keys();
    QJsonArray dynBlocksInfo;
    for (const auto &i : std::as_const(dynBlocksTypes)) {
        dynBlocksInfo.append(m_dataContext.dynamicsBlocksInfo[i].toJson());
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

    m_dataContext.dynamicsBlocksInfo.clear();

    for (const auto &i : std::as_const(dynamicBlocks)) {
        auto data = i.toObject();
        registerBlock(BlockData::fromJson(data), false);
    }
    // qDebug() << "dynamicsBlocksAdded";

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
            auto find = m_dataContext.blocksInfo.find(type);
            if (find != m_dataContext.blocksInfo.end()) {
                if (index >= 0 && index < find->slotsPlaceHolders.count())
                    text = find->slotsPlaceHolders[index];
            }
        } else if (origin == BlockData::Dynamic) {
            auto find = m_dataContext.dynamicsBlocksInfo.find(type);
            if (find != m_dataContext.dynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsPlaceHolders.count())
                    text = find->slotsPlaceHolders[index];
            }
        } else if (origin == BlockData::System) {
            if (type == 0 || type == 1 || type == 2 || type == 3 || type == 6) {
                return "any";
            }

            if (type == 4 || type == 5 || type == 12) {
                return "index";
            }

            if (type == 8) {
                if (index == 0) {
                    return "index";
                } else {
                    return "any";
                }
            }

            if (type == 9) {
                if (index == 0) {
                    return "key";
                } else {
                    return "any";
                }
            }

            if (type == 13 || type == 14) {
                return "key";
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
            auto find = m_dataContext.blocksInfo.find(type);

            if (find != m_dataContext.blocksInfo.end()) {
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
            auto find = m_dataContext.dynamicsBlocksInfo.find(type);

            if (find != m_dataContext.dynamicsBlocksInfo.end()) {
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
            auto find = m_dataContext.blocksInfo.find(type);

            if (find != m_dataContext.blocksInfo.end()) {
                if (index >= 0 && index < find->buttonSettersNewValue.count()) {
                    newValue = find->buttonSettersNewValue[index]({lastKey, lastValue});
                }
            }
        } else {
            auto find = m_dataContext.dynamicsBlocksInfo.find(type);

            if (find != m_dataContext.dynamicsBlocksInfo.end()) {
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
    } else if (method == "removeDynamicBlock") {
        m_dataContext.dynamicsBlocksInfo.remove(data.toInteger());
    }
    return {};
}

int EditorWatcher::getUniqDynamicBlockType(int id)
{
    while (m_dataContext.dynamicsBlocksInfo.contains(id)) {
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
    data.slotsPlaceHolders.append(placeholder);
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
