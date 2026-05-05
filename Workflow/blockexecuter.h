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

    int containerSelection = 0;
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
    Argument(DataContext *context,
             BaseWorkFlow *workFlow,
             int type,
             std::function<QVariant()> getter)
        : context{context}
        , workFlow{workFlow}
        , m_type{(Argument::Type) type}
        , getter{getter}
    {}
    enum Type { Value, Reporter, ComboBox, Button };
    operator QVariant() { return value(); }

    QString name;
    QVariant value() const { return getter(); }
    Type type() const;

private:
    DataContext *context;
    BaseWorkFlow *workFlow;
    Type m_type = Value;
    std::function<QVariant()> getter;
};

struct SlotsData
{
    QJsonArray slots_;
    QJsonArray comboBoxs_;
    QJsonArray buttons_;
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

    virtual ExecuteResult exec(const ExecuteQuery &query, QVariant &returnResult) { return {}; }

    BaseWorkFlow *workFlow;
    DataContext *context;

    // Является ли блок возвращающим значение
    bool isReporter = false;
    // Определение пользовательского блока
    bool isDefine = false;
    // Аргумент внутри определения пользовательского блока
    bool isDefineReporter = false;

    int origin = 1; ///< Происхождение блока (подтип)
    int type = 1;   ///< Тип

    QJsonObject tags;

    QList<Chain> containers; ///< Цепочки в контейнерах

    QList<Argument> args; ///< аргументы которые пришли

    void fromJson(const QJsonObject &data);

private:
    /// Это не нужно использовать, это для обработки слотов, используйте args для получения входных значений
    SlotsData slotsData;
};

#endif // BLOCKEXECUTER_H
