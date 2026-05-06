#ifndef RUNEXECUTER_H
#define RUNEXECUTER_H

#include <QObject>
#include <QPointer>
#include "blockexecuter.h"

class RunExecuter : public QObject
{
    Q_OBJECT
public:
    RunExecuter(ChainId id, Chain chain);
    ExecuteResult run();

    ExecuteResult prepareArgs(BlockExecuter *executer);
    ExecuteResult runContainer(const ExecuteResult &result, BlockExecuter *executer);

private:
    ChainId m_id;
    Chain m_chain;

signals:

private slots:
};

#endif // RUNEXECUTER_H
