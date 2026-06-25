#ifndef MYDATACONTEXT_H
#define MYDATACONTEXT_H

#include "datacontext.h"

class MyDataContext : public DataContext
{
    Q_OBJECT
public:
    MyDataContext(QObject *parent = nullptr)
        : DataContext{nullptr}
    {
        int ids = 0;
        registerUserBlock(BlockConstructor{"Networks", ids, false, true, "#bfcdd9"}
                              .text("Upd-сервер: порт")
                              .slot("port 0-65000")
                              .replica("bytesData"));
        ids++;

        registerUserBlock(ReporterConstructor{"Files", ids, "#bfcdd9"}
                              .text("Получить данные из файла")
                              .slot("Путь"));
        ids++;
    }
};

#endif // MYDATACONTEXT_H
