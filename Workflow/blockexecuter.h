#ifndef BLOCKEXECUTER_H
#define BLOCKEXECUTER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

class BaseWorkFlow;
class DataContext;
class BlockExecuter;

using ChainId = qint64;
using Chain = QList<QPointer<BlockExecuter>>;

struct ExecuteResult
{
    enum State { Success, Error };

    State state = Success;
    QString errorMessage;

    QList<int> executableContainers;
    QList<int> executableChains;
};

struct ExecuteQuery
{
    enum Type { Execute, SelectContainer };

    Type type = Execute;
    QString infoMessage;
};

class Argument
{
public:
    Argument(DataContext *context, BaseWorkFlow *workFlow, QVariant value)
        : context{context}
        , workFlow{workFlow}
        , m_value{value}
    {}
    operator QVariant() { return value(); }

    QString name;
    QVariant value() const { return m_value; }

private:
    DataContext *context;
    BaseWorkFlow *workFlow;
    QVariant m_value;
};

struct SlotsData
{
    QList<QPair<int, QJsonValue>> slots_;
};

class BlockExecuter : public QObject
{
    Q_OBJECT

    friend class BaseWorkFlow;
    friend class RunExecuter;

public:
    BlockExecuter(DataContext *context, BaseWorkFlow *workFlow)
        : context{context}
        , workFlow{workFlow}
    {}
    ~BlockExecuter();

    virtual ExecuteResult exec(QVariant &returnResult);

    BaseWorkFlow *workFlow;
    DataContext *context;

    // Является ли блок возвращающим значение
    bool isReporter = false;
    // Определение пользовательского блока
    bool isDefine = false;
    // Аргумент внутри определения пользовательского блока
    bool isDefineReporter = false;

    /// Происхождение блока (подтип) 0 это кастомный, 1 - пользовательский блок, 2 - системный
    int origin = 1;
    int type = 1;   ///< Тип

    QJsonObject tags;

    QList<Chain> containers; ///< Цепочки в контейнерах

    QList<Argument> args; ///< аргументы которые пришли

    void fromJson(const QJsonObject &data);

private:
    /// Это не нужно использовать, это для обработки слотов, используйте args для получения входных значений
    SlotsData slotsData;

    QList<QString> argsNames;
};

class DymanicBlock : public BlockExecuter
{
    Q_OBJECT
public:
    DymanicBlock(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override;
};

#endif // BLOCKEXECUTER_H
