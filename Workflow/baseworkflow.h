#ifndef BASEWORKFLOW_H
#define BASEWORKFLOW_H

#include <QJsonArray>
#include <QPointer>

#include "blockexecuter.h"
#include "datacontext.h"

using TypeId = QString;
using Creator = std::function<QPointer<BlockExecuter>()>;

class BaseWorkFlow
{
    friend class BlockExecuter;

public:
    BaseWorkFlow(DataContext *context);
    ~BaseWorkFlow();

    // Загрузка скрипта
    bool loadScript(const QJsonObject &script);

    // Регистрация начинки блока
    template<class T>
    void registerBlock(qint64 origin, qint64 type)
    {
        TypeId typeId = QString::number(origin) + "_" + QString::number(type);
        factory[typeId] = [this]() { return QPointer<T>(new T(context, this)); };
    }

    // Создание задачи из пула зарегистрированных
    QPointer<BlockExecuter> createExecuter(const QJsonObject &data);

    // Служебный метод для создания цепочек из массива Json
    Chain createChain(const QJsonArray &chain);

    // Получение цепочек у которых начальный блок с указанными типами, origin = -1 -> вернет все цепочки
    QList<ChainId> getChainWithType(int origin = -1, int type = -1) const;

    Chain getChainWithId(ChainId id) const;

    void runChain(ChainId id);

private:
    DataContext *context = nullptr;

    QMap<varName, ctxVariable> *variables = nullptr;
    QMap<ChainId, Chain> chains;

    QMap<TypeId, Creator> factory;

    QPointer<BlockExecuter> createDymanicExecuter();
    void registerStdExecuters();
};

#endif // BASEWORKFLOW_H
