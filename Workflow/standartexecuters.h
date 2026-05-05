#include "blockexecuter.h"

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

} // namespace StdExcts
