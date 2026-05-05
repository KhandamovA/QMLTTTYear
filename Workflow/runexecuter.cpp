#include "runexecuter.h"
#include "baseworkflow.h"

RunExecuter::RunExecuter(ChainId id, Chain chain)
    : m_id{id}
    , m_chain{chain}
{}

void RunExecuter::run()
{
    for (auto &i : m_chain) {
        // Подготовливаем аргументы обрабатывая слоты
        prepareArgs(i);

        QVariant returnResult;
        auto executeResult = i.get()->exec(ExecuteQuery{}, returnResult);

        if (executeResult.state == ExecuteResult::Error) {
            qWarning() << "ErrorMsg:" << executeResult.errorMessage;
            break;
        }
    }
}

void RunExecuter::prepareArgs(BlockExecuter *executer)
{
    auto &args = executer->args;
    auto &slotsData = executer->slotsData;
    auto workFlow = executer->workFlow;
    auto context = executer->context;
    args.clear();

    for (const auto &i : slotsData.slots_) {
        if (i.isObject()) {
            QVariant returnResult;
            auto reporter = workFlow->createExecuter(i.toObject());
            prepareArgs(reporter);
            auto execResult = reporter->exec({}, returnResult);

            Argument arg(context, workFlow, Argument::Value, returnResult);
            args.append(arg);
        } else {
            Argument arg(context, workFlow, Argument::Value, i.toVariant());
            args.append(arg);
        }
    }
}
