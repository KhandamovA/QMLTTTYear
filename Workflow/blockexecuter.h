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
    bool exit = false;
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
    QVariant &operator=(const QVariant &other)
    {
        m_value = other;
        return m_value;
    }

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
    BlockExecuter(DataContext *context, BaseWorkFlow *workFlow);
    ~BlockExecuter();

    virtual ExecuteResult exec(QVariant &returnResult);

    BaseWorkFlow *workFlow;
    DataContext *context;

    //Блок в котором он лежит
    BlockExecuter *parent = nullptr;

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

    qint64 chainId() const;
    void setChainId(qint64 newChainId);

private:
    /// Это не нужно использовать, это для обработки слотов, используйте args для получения входных значений
    SlotsData slotsData;

    qint64 m_chainId = 0; ///< Цепочка в которой тот находится

    QList<QString> argsNames;
};

class DynamicBlock : public BlockExecuter
{
    Q_OBJECT
public:
    DynamicBlock(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override;
};

class DynamicBlockReturn : public BlockExecuter
{
    Q_OBJECT
public:
    DynamicBlockReturn(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override;
};

#endif // BLOCKEXECUTER_H
