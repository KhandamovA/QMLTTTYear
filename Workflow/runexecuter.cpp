#include "runexecuter.h"
#include "baseworkflow.h"

RunExecuter::RunExecuter(ChainId id, Chain chain)
    : m_id{id}
    , m_chain{chain}
{}

void RunExecuter::run()
{
    ExecuteResult result;

    for (auto &i : m_chain) {
        // Подготовливаем аргументы обрабатывая слоты
        result = prepareArgs(i);

        if (result.state == ExecuteResult::Error) {
            qWarning() << "ErrorMsg:" << result.errorMessage;
            break;
        }

        QVariant returnResult;
        result = i.get()->exec(ExecuteQuery{}, returnResult);

        if (result.state == ExecuteResult::Error) {
            qWarning() << "ErrorMsg:" << result.errorMessage;
            break;
        }
    }

    deleteLater();
}

ExecuteResult RunExecuter::prepareArgs(BlockExecuter *executer)
{
    ExecuteResult result;

    auto &args = executer->args;
    auto &slotsData = executer->slotsData;
    auto workFlow = executer->workFlow;
    auto context = executer->context;
    args.clear();

    for (const auto &[type, data] : std::as_const(slotsData.slots_)) {
        // Обработка обычных слотов
        if (type == slotInfo::Plain) {
            if (data.isObject()) {
                // Если в слоте есть репортер
                QVariant returnResult;
                auto reporter = workFlow->createExecuter(data.toObject());
                result = prepareArgs(reporter);

                if (result.state == ExecuteResult::Error) {
                    return result;
                }

                result = reporter->exec({}, returnResult);

                if (result.state == ExecuteResult::Error) {
                    return result;
                }

                // Добавляем готовый аргумент
                Argument arg(context, workFlow, Argument::PlainValue, returnResult);
                args.append(arg);
            } else {
                // Если в слоте репортера нет
                // Добавляем готовый аргумент
                Argument arg(context, workFlow, Argument::PlainValue, data.toVariant());
                args.append(arg);
            }
        }
    }

    return result;
}
