#include "baseworkflow.h"
#include "runexecuter.h"
#include "standartexecuters.h"

#include <QJsonObject>

BaseWorkFlow::BaseWorkFlow(DataContext *context)
    : context{context}
{
    variables = &context->variables;
    registerStdExecuters();
}

BaseWorkFlow::~BaseWorkFlow()
{
    for (auto &i : chains) {
        qDeleteAll(i);
    }
}

bool BaseWorkFlow::loadScript(const QJsonObject &script)
{
    for (auto &i : chains) {
        qDeleteAll(i);
    }
    chains.clear();
    variables->clear();
    context->dynamicsBlocksInfo.clear();

    // Загрузка переменных
    if (script.contains("variables")) {
        auto variables_ = script["variables"].toArray();
        for (const auto &i : std::as_const(variables_)) {
            auto name = i.toString();
            (*variables)[name] = "";
            (*variables)[name].name = name;
        }
    } else {
        qWarning()
            << "Скрипт который вы пытаетесь загрузить не содержит поля variables (переменные)";
        return false;
    }

    // Загрузка пользовательских блоков
    if (script.contains("dynamicBlocks")) {
        auto blocks = script["dynamicBlocks"].toArray();
        for (const auto &i : std::as_const(blocks)) {
            auto block = i.toObject();
            auto blockStruct = BlockData::fromJson(block);
            context->dynamicsBlocksInfo[blockStruct.type] = blockStruct;
        }
    } else {
        qWarning() << "Скрипт который вы пытаетесь загрузить не содержит поля dynamicBlocks "
                      "(пользовательские блоки)";
        return false;
    }

    // Загрузка цепочек скриптов
    if (script.contains("chains")) {
        qint64 ids = 0;
        for (const auto &i : script["chains"].toArray()) {
            Chain ch = createChain(i.toArray(), ids);
            chains[ids] = ch;
            ids++;
        }
    } else {
        qWarning() << "Скрипт который вы пытаетесь загрузить не содержит поля chains (цепочки "
                      "последовательностей)";
        return false;
    }

    return true;
}

QPointer<BlockExecuter> BaseWorkFlow::createExecuter(const QJsonObject &data)
{
    auto origin = data["origin"].toInteger();
    auto type = data["type"].toInteger();

    // Обработка динамических блоков
    if (origin == BlockData::Dynamic) {
        auto block = createDymanicExecuter();
        block.get()->fromJson(data);
        return block;
    } else {
        TypeId typeId = QString::number(origin) + "_" + QString::number(type);
        // qDebug() << factory.keys();
        auto fac = factory.find(typeId);
        if (fac != factory.end()) {
            auto block = fac.value()();
            // Подгрузка данных
            block.get()->fromJson(data);
            return block;
        } else {
            qWarning() << "Executer с происхождением" << origin << "и типом" << type
                       << "не зарегистрирован и не может быть создан";
        }
    }
    return nullptr;
}

Chain BaseWorkFlow::createChain(const QJsonArray &chain, qint64 chainId)
{
    Chain ret;

    for (const auto &i : chain) {
        auto ctx = createExecuter(i.toObject());
        ctx->setChainId(chainId);
        ret.append(ctx);
    }

    return ret;
}

QList<ChainId> BaseWorkFlow::getChainWithType(int origin, int type) const
{
    QList<ChainId> ret;
    auto keys = chains.keys();
    for (auto &i : keys) {
        auto ch = chains[i];
        auto first = chains[i][0];
        if (origin == -1 || (first.get()->origin == origin && first.get()->type == type)) {
            ret.append(i);
        }
    }
    return ret;
}

Chain BaseWorkFlow::getChainWithId(ChainId id) const
{
    auto find = chains.find(id);
    if (find != chains.end()) {
        return (*find);
    } else {
        qWarning() << "Не удалось найти цепочку c id" << id;
    }
    return {};
}

void BaseWorkFlow::runChain(ChainId id)
{
    auto chain = getChainWithId(id);

    if (chain.count() == 0) {
        qWarning() << "Не удалось запустить цепочку с id" << id << "т.к. в цепочке нет блоков";
        return;
    }

    auto executer = new RunExecuter(id, chain);
    executer->run();
}

QPointer<BlockExecuter> BaseWorkFlow::createDymanicExecuter()
{
    auto block = QPointer<BlockExecuter>(new DynamicBlock(context, this));
    return block;
}

void BaseWorkFlow::registerStdExecuters()
{
    registerBlock<DynamicBlockReturn>(BlockData::System, 500);

    registerBlock<StdExcts::WhenScriptLoaded>(BlockData::System, 6);
    registerBlock<StdExcts::IfElse>(BlockData::System, 2);
    registerBlock<StdExcts::Debug>(BlockData::System, 4);

    registerBlock<StdExcts::VariableGet>(BlockData::System, 100);
    registerBlock<StdExcts::VariableSet>(BlockData::System, 101);

    registerBlock<StdExcts::operators::Plus>(BlockData::System, 200);
    registerBlock<StdExcts::operators::Minus>(BlockData::System, 201);
    registerBlock<StdExcts::operators::Glue>(BlockData::System, 206);
}
