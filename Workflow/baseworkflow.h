#ifndef BASEWORKFLOW_H
#define BASEWORKFLOW_H

#include <QObject>

class DataContext;

class BaseWorkFlow : public QObject
{
    Q_OBJECT
public:
    BaseWorkFlow(DataContext *context) {}
};

#endif // BASEWORKFLOW_H
