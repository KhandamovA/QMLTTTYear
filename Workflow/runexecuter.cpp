#include "runexecuter.h"
#include "baseworkflow.h"

RunExecuter::RunExecuter(ChainId id, Chain chain)
    : m_id{id}
    , m_chain{chain}
{}

ExecuteResult RunExecuter::run(bool withDelete)
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
        result = i.get()->exec(returnResult);

        if (result.state == ExecuteResult::Error) {
            qWarning() << "ErrorMsg:" << result.errorMessage;
            break;
        }

        // Сигнал досрочного выхода из цепочки
        if (result.exit) {
            break;
        }

        result = runContainer(result, i);

        // Сигнал досрочного выхода из цепочки
        if (result.exit) {
            break;
        }

        if (result.state == ExecuteResult::Error) {
            qWarning() << "ErrorMsg:" << result.errorMessage;
            break;
        }
    }

    if (withDelete) {
        deleteLater();
    }
    return result;
}

ExecuteResult RunExecuter::prepareArgs(BlockExecuter *executer)
{
    ExecuteResult result;

    auto tagsKeys = executer->tags.keys();
    auto &args = executer->args;
    auto &slotsData = executer->slotsData;
    auto workFlow = executer->workFlow;
    auto context = executer->context;

    //У определения аргументы трогать нельзя
    if (!tagsKeys.contains("define")) {
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

                    result = reporter->exec(returnResult);

                    if (result.state == ExecuteResult::Error) {
                        return result;
                    }

                    // Добавляем готовый аргумент
                    Argument arg(context, workFlow, returnResult);
                    arg.name = executer->argsNames[args.count()];
                    args.append(arg);

                    reporter->deleteLater();
                } else {
                    // Если в слоте репортера нет
                    // Добавляем готовый аргумент
                    Argument arg(context, workFlow, data.toVariant());
                    arg.name = executer->argsNames[args.count()];
                    args.append(arg);
                }
            } else if (type == slotInfo::ComboBox || type == slotInfo::Button) {
                auto data_ = data.toObject();
                auto value = data_["value"];
                Argument arg(context, workFlow, value.toVariant());
                arg.name = executer->argsNames[args.count()];
                args.append(arg);
            } else {
                qWarning() << type << "Обработчика данного вида слота не существует";
            }
        }
    }

    return result;
}

ExecuteResult RunExecuter::runContainer(const ExecuteResult &result, BlockExecuter *executer)
{
    ExecuteResult ret;
    for (const auto &i : result.executableContainers) {
        auto chain = executer->containers[i];

        auto executer = new RunExecuter(m_id, chain);
        ret = executer->run();

        if (ret.state == ExecuteResult::Error) {
            break;
        }
    }
    return ret;
}
