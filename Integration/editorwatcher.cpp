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

void EditorWatcher::init()
{
    registerBlocks(m_dataContext.systemBlocksInfo.values());
}

void EditorWatcher::registerBlock(BlockData data, bool checkDefine)
{
    if (data.origin == 1) {
        data.type = getUniqDynamicBlockType(data.type);
        m_dataContext.dynamicsBlocksInfo[data.type] = data;
    } else if (data.origin == 2) {
        m_dataContext.systemBlocksInfo[data.type] = data;
    } else {
        m_dataContext.blocksInfo[data.type] = data;
    }
    sendCommand("registerBlock", QJsonObject{{"data", data.toJson()}, {"checkDefine", checkDefine}});
}

void EditorWatcher::registerBlocks(QList<BlockData> data, bool checkDefine)
{
    QJsonArray data_;
    for (auto &i : data) {
        if (i.origin == 1) {
            i.type = getUniqDynamicBlockType(i.type);
            m_dataContext.dynamicsBlocksInfo[i.type] = i;
        } else if (i.origin == 2) {
            m_dataContext.systemBlocksInfo[i.type] = i;
        } else {
            m_dataContext.blocksInfo[i.type] = i;
        }
        data_.append(i.toJson());
    }

    sendCommand("registerBlocks", QJsonObject{{"data", data_}, {"checkDefine", checkDefine}});
}

QJsonObject EditorWatcher::getSceneBlockData(qint64 uid)
{
    auto data = sendCommand("getSceneBlockData", uid);
    return data.toObject();
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
    auto blockUid = data["blockUid"].toInteger();

    if (method == "slotPlaceholder") {
        auto type = data["type"].toInteger();
        auto index = data["index"].toInteger();
        auto origin = data["origin"].toInt();

        QString text;
        if (origin == BlockData::Custom) {
            auto find = m_dataContext.blocksInfo.find(type);
            if (find != m_dataContext.blocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count())
                    text = find->slotsInfo[index].name;
            }
        } else if (origin == BlockData::Dynamic) {
            auto find = m_dataContext.dynamicsBlocksInfo.find(type);
            if (find != m_dataContext.dynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count())
                    text = find->slotsInfo[index].name;
            }
        } else if (origin == BlockData::System) {
            auto find = m_dataContext.systemBlocksInfo.find(type);
            if (find != m_dataContext.systemBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count())
                    text = find->slotsInfo[index].name;
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

        auto toArray = [](QList<QPair<QString, QVariant>> list) {
            QJsonArray ret;
            for (auto &i : list) {
                QJsonObject entry;
                entry["key"] = i.first;
                entry["value"] = i.second.toJsonValue();
                ret.append(entry);
            }
            return ret;
        };

        if (origin == BlockData::Custom) {
            auto find = m_dataContext.blocksInfo.find(type);

            if (find != m_dataContext.blocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count()) {
                    auto list = find->slotsInfo[index].getterList({lastKey, lastValue}, blockUid);
                    ret = toArray(list);
                }
            }
        } else if (origin == BlockData::Dynamic) {
            auto find = m_dataContext.dynamicsBlocksInfo.find(type);

            if (find != m_dataContext.dynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count()) {
                    auto list = find->slotsInfo[index].getterList({lastKey, lastValue}, blockUid);
                    ret = toArray(list);
                }
            }
        } else if (origin == BlockData::System) {
            auto find = m_dataContext.systemBlocksInfo.find(type);

            if (find != m_dataContext.systemBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count()) {
                    auto list = find->slotsInfo[index].getterList({lastKey, lastValue}, blockUid);
                    ret = toArray(list);
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
        auto origin = data["origin"].toInt();
        QPair<QString, QVariant> newValue;

        if (origin == BlockData::Custom) {
            auto find = m_dataContext.blocksInfo.find(type);

            if (find != m_dataContext.blocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count()) {
                    newValue = find->slotsInfo[index].getterButtonValue({lastKey, lastValue},
                                                                        blockUid);
                }
            }
        } else if (origin == BlockData::Dynamic) {
            auto find = m_dataContext.dynamicsBlocksInfo.find(type);

            if (find != m_dataContext.dynamicsBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count()) {
                    newValue = find->slotsInfo[index].getterButtonValue({lastKey, lastValue},
                                                                        blockUid);
                }
            }
        } else if (origin == BlockData::System) {
            auto find = m_dataContext.systemBlocksInfo.find(type);

            if (find != m_dataContext.systemBlocksInfo.end()) {
                if (index >= 0 && index < find->slotsInfo.count()) {
                    newValue = find->slotsInfo[index].getterButtonValue({lastKey, lastValue},
                                                                        blockUid);
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

DataContext *EditorWatcher::dataContext()
{
    return &m_dataContext;
}

void EditorWatcher::createNewBlock()
{
    BlockEditor editor(this);
    editor.exec();

    if (editor.isAccepted()) {
        auto data = editor.save();
        auto viewText = data.viewTexts[0].trimmed();
        data.type = m_dataContext.dynamicsBlocksInfo.count();
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
