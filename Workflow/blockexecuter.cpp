#include "blockexecuter.h"
#include <QJsonArray>
#include "baseworkflow.h"
#include "datacontext.h"
#include "runexecuter.h"

BlockExecuter::~BlockExecuter()
{
    for (auto &i : containers) {
        qDeleteAll(i);
    }
}

ExecuteResult BlockExecuter::exec(QVariant &returnResult)
{
    return {};
}

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

        argsNames.append(blockInfo.slotsInfo[slotsData.slots_.count()].name);
        slotsData.slots_.append({obj["type"].toInt(), obj["data"]});
    }
}

ExecuteResult DymanicBlock::exec(QVariant &returnResult)
{
    ExecuteResult result;

    auto tagsKeys = tags.keys();

    if (tagsKeys.contains("define")) {
        return result;
    } else if (tagsKeys.contains("replica")) {
        auto define_ = workFlow->getChainWithType(BlockData::Dynamic, this->type);
        if (define_.count() > 0) {
            auto chainId = define_.first();
            auto chain = workFlow->getChainWithId(chainId);
            auto &firstBlock = chain[0];
            auto slotName = tags["slotName"].toString();

            bool success = false;
            // Заполняем выходное значение
            for (const auto &i : firstBlock->args) {
                if (i.name == slotName) {
                    success = true;
                    returnResult = i.value();
                    break;
                }
            }

            if (!success) {
                qWarning()
                    << "Не удалось получить значение из определителя пользовательского блока";
            }
        } else {
            qWarning() << "Отсутствует определение для пользовательского блока" << this->type;
        }
    } else {
        // Основная логика
        // Находим и запускаем цепочку
        auto define_ = workFlow->getChainWithType(BlockData::Dynamic, this->type);
        if (define_.count() > 0) {
            auto chainId = define_.first();
            auto chain = workFlow->getChainWithId(chainId);
            auto &firstBlock = chain[0];
            // Заполняем входные аргументы

            firstBlock->args.clear();
            for (const auto &i : this->args) {
                firstBlock->args.append(i);
                firstBlock->args.last().name = i.name;
            }

            RunExecuter *executer = new RunExecuter(chainId, chain);
            result = executer->run();
        } else {
            qWarning() << "Отсутствует определение для пользовательского блока" << this->type;
        }
    }

    return result;
}
