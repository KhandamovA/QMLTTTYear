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
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
    {
        ExecuteResult result;
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
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
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

class Debug : public BlockExecuter
{
    Q_OBJECT
public:
    Debug(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
    {
        ExecuteResult result;

        QVariant text = args[0];

        qDebug() << text.toString();

        return result;
    }
};

class VariableGet : public BlockExecuter
{
    Q_OBJECT
public:
    VariableGet(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();

        returnResult = context->variables[varName];

        return result;
    }
};

class VariableSet : public BlockExecuter
{
    Q_OBJECT
public:
    VariableSet(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
    {
        ExecuteResult result;

        auto varName = args[0].value().toString();
        QVariant newValue = args[1];

        context->variables[varName] = newValue;

        return result;
    }
};

namespace operators {

class Plus : public BlockExecuter
{
    Q_OBJECT
public:
    Plus(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

public:
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
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
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toDouble();
        auto second = args[1].value().toDouble();
        returnResult = first - second;
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
    ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) override
    {
        ExecuteResult result;
        auto first = args[0].value().toString();
        auto second = args[1].value().toString();
        returnResult = QString(first + second);
        return result;
    }
};

} // namespace operators

} // namespace StdExcts
