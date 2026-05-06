#include "Workflow/runexecuter.h"
#include "blockexecuter.h"
#include "datacontext.h"

namespace StdExcts {

class WhenScriptLoaded : public BlockExecuter
{
    Q_OBJECT
public:
    WhenScriptLoaded(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        return result;
    }
};

class While : public BlockExecuter
{
    Q_OBJECT
public:
    While(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        RunExecuter *executer = new RunExecuter(chainId(), containers[0]);

        while (true) {
            // Обновление местных аргументов
            executer->prepareArgs(this);

            // Проверка условия
            auto condition = args[0].value().toDouble();
            if (condition) {
                // Выполнение тела цикла
                result = executer->run(false);

                if (result.state == ExecuteResult::Error || result.exit) {
                    break;
                }
            } else {
                //Выходим из цикла
                break;
            }
        }

        executer->deleteLater();
        return result;
    }
};

class WhileCount : public BlockExecuter
{
    Q_OBJECT
public:
    WhileCount(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        RunExecuter *executer = new RunExecuter(chainId(), containers[0]);

        auto count = args[0].value().toInt();

        for (int i = 0; i < count; i++) {
            // Выполнение тела цикла
            result = executer->run(false);

            if (result.state == ExecuteResult::Error || result.exit) {
                break;
            }
        }

        executer->deleteLater();
        return result;
    }
};

class IfElse : public BlockExecuter
{
    Q_OBJECT
public:
    IfElse(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto condition = args[0].value().toDouble();

        if (condition) {
            // Дает добро на запуск первого контейнера
            result.executableContainers.append(0);
        } else {
            // Дает добро на запуск второго контейнера с иначе
            result.executableContainers.append(1);
        }

        return result;
    }
};

class If : public BlockExecuter
{
    Q_OBJECT
public:
    If(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto condition = args[0].value().toDouble();

        if (condition) {
            // Дает добро на запуск первого контейнера
            result.executableContainers.append(0);
        }

        return result;
    }
};

class Debug : public BlockExecuter
{
    Q_OBJECT
public:
    Debug(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        QVariant text = args[0];

        qDebug() << text.toString();

        return result;
    }
};

namespace Variable {

class Get : public BlockExecuter
{
    Q_OBJECT
public:
    Get(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        returnResult = var;

        return result;
    }
};

class Set : public BlockExecuter
{
    Q_OBJECT
public:
    Set(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];
        QVariant newValue = args[1];

        var = newValue;

        return result;
    }
};

} // namespace Variable

namespace Array {

class Clear : public BlockExecuter
{
    Q_OBJECT
public:
    Clear(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        var.clear();

        return result;
    }
};

class Add : public BlockExecuter
{
    Q_OBJECT
public:
    Add(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        var.append(args[1]);

        return result;
    }
};

class RemoveAt : public BlockExecuter
{
    Q_OBJECT
public:
    RemoveAt(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        var.removeAt(args[1].value().toDouble());

        return result;
    }
};

class GetItem : public BlockExecuter
{
    Q_OBJECT
public:
    GetItem(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[1].value().toString();
        auto &var = context->variables[varName];

        returnResult = var[args[0].value().toDouble()];

        return result;
    }
};

class IndexOf : public BlockExecuter
{
    Q_OBJECT
public:
    IndexOf(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[1].value().toString();
        auto &var = context->variables[varName];

        returnResult = var.indexOf(args[0]);

        return result;
    }
};

class Count : public BlockExecuter
{
    Q_OBJECT
public:
    Count(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        returnResult = var.count();

        return result;
    }
};

class Replace : public BlockExecuter
{
    Q_OBJECT
public:
    Replace(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        auto index = args[1].value().toDouble();

        if (!(index < 0 || index >= var.count())) {
            var[index] = args[2];
        }

        return result;
    }
};

} // namespace Array

namespace Map {

class Set : public BlockExecuter
{
    Q_OBJECT
public:
    Set(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        var.setValue(args[1].value().toString(), args[2]);

        return result;
    }
};

class Clear : public BlockExecuter
{
    Q_OBJECT
public:
    Clear(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        var.clear();

        return result;
    }
};

class CountKeys : public BlockExecuter
{
    Q_OBJECT
public:
    CountKeys(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        returnResult = var.keys().count();

        return result;
    }
};

class KeyAt : public BlockExecuter
{
    Q_OBJECT
public:
    KeyAt(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[1].value().toString();
        auto &var = context->variables[varName];
        auto index = args[0].value().toDouble();

        if (index < 0 || index >= var.keys().count()) {
            returnResult = var.keys().at(index);
        }

        return result;
    }
};

class Get : public BlockExecuter
{
    Q_OBJECT
public:
    Get(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[1].value().toString();
        auto &var = context->variables[varName];

        returnResult = var.value(args[0].value().toString());

        return result;
    }
};

class RemoveKey : public BlockExecuter
{
    Q_OBJECT
public:
    RemoveKey(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        auto &var = context->variables[varName];

        var.removeKey(args[1].value().toString());

        return result;
    }
};

} // namespace Map

namespace operators {

class Plus : public BlockExecuter
{
    Q_OBJECT
public:
    Plus(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first + second;
        return result;
    }
};

class Minus : public BlockExecuter
{
    Q_OBJECT
public:
    Minus(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first - second;
        return result;
    }
};

class Multiply : public BlockExecuter
{
    Q_OBJECT
public:
    Multiply(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first * second;
        return result;
    }
};

class Divide : public BlockExecuter
{
    Q_OBJECT
public:
    Divide(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first / second;
        return result;
    }
};

class Power : public BlockExecuter
{
    Q_OBJECT
public:
    Power(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = std::pow(first, second);
        return result;
    }
};

class Modulo : public BlockExecuter
{
    Q_OBJECT
public:
    Modulo(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = static_cast<qint64>(args[0].value().toDouble());
        auto second = static_cast<qint64>(args[1].value().toDouble());
        returnResult = first % second;
        return result;
    }
};

// Склеить две строки
class Glue : public BlockExecuter
{
    Q_OBJECT
public:
    Glue(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toString();
        auto second = args[1].value().toString();
        returnResult = QString(first + second);
        return result;
    }
};

class Equal : public BlockExecuter
{
    Q_OBJECT
public:
    Equal(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first == second;
        return result;
    }
};

class NotEqual : public BlockExecuter
{
    Q_OBJECT
public:
    NotEqual(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first != second;
        return result;
    }
};

class More : public BlockExecuter
{
    Q_OBJECT
public:
    More(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first > second;
        return result;
    }
};

class Less : public BlockExecuter
{
    Q_OBJECT
public:
    Less(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first < second;
        return result;
    }
};

class MoreEqual : public BlockExecuter
{
    Q_OBJECT
public:
    MoreEqual(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first >= second;
        return result;
    }
};

class LessEqual : public BlockExecuter
{
    Q_OBJECT
public:
    LessEqual(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first <= second;
        return result;
    }
};

class And : public BlockExecuter
{
    Q_OBJECT
public:
    And(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first && second;
        return result;
    }
};

class Or : public BlockExecuter
{
    Q_OBJECT
public:
    Or(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first || second;
        return result;
    }
};

class Not : public BlockExecuter
{
    Q_OBJECT
public:
    Not(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        returnResult = !first;
        return result;
    }
};

class Negative : public BlockExecuter
{
    Q_OBJECT
public:
    Negative(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        returnResult = -first;
        return result;
    }
};

} // namespace operators

} // namespace StdExcts
