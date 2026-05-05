#include "blockexecuter.h"
#include <QJsonArray>
#include "baseworkflow.h"
#include "datacontext.h"

void BlockExecuter::fromJson(const QJsonObject &data)
{
    origin = data["origin"].toInteger();
    type = data["type"].toInteger();

    tags = data["tags"].toObject();

    auto blockInfo = context->getBlockInfo(origin, type);

    if (blockInfo.blockShape == BlockData::Reporter) {
        isReporter = true;
    }

    if (tags.contains("define")) {
        isDefine = true;
    }

    if (tags.contains("replica")) {
        isDefineReporter = true;
    }

    // Заполняем контейнеры
    auto containers = data["containers"].toArray();
    for (const auto &i : std::as_const(containers)) {
        this->containers.append(workFlow->createChain(i.toArray()));
    }

    auto s = data["slots"].toArray();
    slotsData.slots_.clear();
    for (const auto &i : std::as_const(s)) {
        auto obj = i.toObject();
        slotsData.slots_.append({obj["type"].toInt(), obj["data"]});
    }
}

Argument::Type Argument::type() const
{
    return m_type;
}
