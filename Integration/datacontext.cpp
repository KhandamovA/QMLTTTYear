#include "datacontext.h"

#include <QJsonArray>

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
{}
