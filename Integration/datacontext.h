#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include <QObject>
#include <QVariant>

using varName = QString;

class ctxVariable
{
public:
    QString name;

    // Методы работы с переменной
    QVariant &operator=(const QVariant &other)
    {
        value = other;
        return value;
    }
    operator QVariant() { return value; }

    // Методы работы с листом
    void append(const QVariant &item) { list.append(item); }
    QVariant operator[](int index) { return list[index]; }
    int count() const { return list.count(); }
    void clear()
    {
        list.clear();
        value = QVariant();
    }
    int indexOf(const QVariant &item) { return list.indexOf(item); }
    int typeId() { return list.count() ? QMetaType::Type::QVariantList : value.typeId(); }

private:
    QVariant value;
    QList<QVariant> list;
};

class DataContext : public QObject
{
    Q_OBJECT
public:
    explicit DataContext(QObject *parent = nullptr);

    QMap<varName, ctxVariable> variables;
};

#endif // DATACONTEXT_H
