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
    if (context) {
        delete context;
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
            Chain ch = createChain(i.toArray(), ids, nullptr);
            chains[ids] = ch;
            ids++;
        }
    } else {
        qWarning() << "Скрипт который вы пытаетесь загрузить не содержит поля chains (цепочки "
                      "последовательностей)";
        return false;
    }

    // Запуск скриптов которые триггерятся при загрузке скрипта
    auto chainsLoadedScript = getChainWithType(BlockData::System, 0);
    for (const auto &i : std::as_const(chainsLoadedScript)) {
        runChain(i);
    }

    return true;
}

QPointer<BlockExecuter> BaseWorkFlow::createExecuter(const QJsonObject &data)
{
    auto origin = data["origin"].toInteger();
    auto type = data["type"].toInteger();
    auto tagsKeys = data["tags"].toObject().keys();

    // Обработка динамических блоков и реплик
    if (origin == BlockData::Dynamic || tagsKeys.contains("replica")) {
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

Chain BaseWorkFlow::createChain(const QJsonArray &chain, qint64 chainId, BlockExecuter *parent)
{
    Chain ret;

    for (const auto &i : chain) {
        auto ctx = createExecuter(i.toObject());
        ctx->parent = parent;
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
        auto first = ch[0];
        if (origin == -1 || (first.get()->origin == origin && first.get()->type == type)) {
            ret.append(i);
        }
    }
    return ret;
}

Chain *BaseWorkFlow::getChainWithId(ChainId id)
{
    auto find = chains.find(id);
    if (find != chains.end()) {
        return &(find.value());
    } else {
        qWarning() << "Не удалось найти цепочку c id" << id;
    }
    return nullptr;
}

void BaseWorkFlow::runChain(ChainId id)
{
    auto chain = getChainWithId(id);

    if (chain->count() == 0) {
        qWarning() << "Не удалось запустить цепочку с id" << id << "т.к. в цепочке нет блоков";
        return;
    }

    auto executer = new RunExecuter(id, *chain);
    executer->run();
}

QPointer<BlockExecuter> BaseWorkFlow::createDymanicExecuter()
{
    auto block = QPointer<BlockExecuter>(new DynamicBlock(context, this));
    return block;
}

void BaseWorkFlow::registerStdExecuters()
{
    // Спец блок для возврата значений внутри пользовательских блоков
    registerBlock<DynamicBlockReturn>(BlockData::System, 500);

    // Управление
    registerBlock<StdExcts::WhenScriptLoaded>(BlockData::System, 0);
    registerBlock<StdExcts::While>(BlockData::System, 1);
    registerBlock<StdExcts::WhileCount>(BlockData::System, 2);
    registerBlock<StdExcts::IfElse>(BlockData::System, 3);
    registerBlock<StdExcts::If>(BlockData::System, 4);
    registerBlock<StdExcts::Debug>(BlockData::System, 5);

    // Переменные
    registerBlock<StdExcts::Variable::Get>(BlockData::System, 100);
    registerBlock<StdExcts::Variable::Set>(BlockData::System, 101);

    // Массивы
    registerBlock<StdExcts::Array::Clear>(BlockData::System, 102);
    registerBlock<StdExcts::Array::Add>(BlockData::System, 103);
    registerBlock<StdExcts::Array::RemoveAt>(BlockData::System, 104);
    registerBlock<StdExcts::Array::GetItem>(BlockData::System, 105);
    registerBlock<StdExcts::Array::IndexOf>(BlockData::System, 106);
    registerBlock<StdExcts::Array::Count>(BlockData::System, 107);
    registerBlock<StdExcts::Array::Replace>(BlockData::System, 108);
    registerBlock<StdExcts::Array::ForEach>(BlockData::System, 116);

    // Словари
    registerBlock<StdExcts::Map::Set>(BlockData::System, 109);
    registerBlock<StdExcts::Map::Clear>(BlockData::System, 110);
    registerBlock<StdExcts::Map::CountKeys>(BlockData::System, 111);
    registerBlock<StdExcts::Map::KeyAt>(BlockData::System, 112);
    registerBlock<StdExcts::Map::Get>(BlockData::System, 113);
    registerBlock<StdExcts::Map::RemoveKey>(BlockData::System, 114);
    registerBlock<StdExcts::Map::ForEach>(BlockData::System, 115);

    // Операторы
    registerBlock<StdExcts::operators::Plus>(BlockData::System, 200);
    registerBlock<StdExcts::operators::Minus>(BlockData::System, 201);
    registerBlock<StdExcts::operators::Multiply>(BlockData::System, 202);
    registerBlock<StdExcts::operators::Divide>(BlockData::System, 203);
    registerBlock<StdExcts::operators::Power>(BlockData::System, 204);
    registerBlock<StdExcts::operators::Modulo>(BlockData::System, 205);
    registerBlock<StdExcts::operators::Glue>(BlockData::System, 206);
    registerBlock<StdExcts::operators::Equal>(BlockData::System, 207);
    registerBlock<StdExcts::operators::NotEqual>(BlockData::System, 208);
    registerBlock<StdExcts::operators::More>(BlockData::System, 209);
    registerBlock<StdExcts::operators::Less>(BlockData::System, 210);
    registerBlock<StdExcts::operators::MoreEqual>(BlockData::System, 211);
    registerBlock<StdExcts::operators::LessEqual>(BlockData::System, 212);
    registerBlock<StdExcts::operators::And>(BlockData::System, 213);
    registerBlock<StdExcts::operators::Or>(BlockData::System, 214);
    registerBlock<StdExcts::operators::Not>(BlockData::System, 215);
    registerBlock<StdExcts::operators::Negative>(BlockData::System, 216);
}
