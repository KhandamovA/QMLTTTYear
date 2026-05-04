#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include <QJsonObject>
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

struct BlockData
{
    enum Shape { Block = 0, Reporter = 1 };
    enum Origin { Custom = 0, Dynamic = 1, System = 2 };

    int type = 0;
    int origin = 0;
    QList<QString> viewTexts;
    bool hasInput = true;
    bool hasOutput = true;
    QString textColor = "black";
    QString bodyColor = "#bfcdd9";
    int blockShape = 0; ///< 0 - обычный блок, 1 - репортер
    QJsonObject tags;

    // Подсказки для слотов
    QList<QString> slotsPlaceHolders;
    // Колбэки для получения списка элементов для combobox-сов
    QList<std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant> lastValue)>>
        comboBoxCallCurrentList;
    // Колбэки для получения установки значения и текста в слот с кнопкой, lastValue приходит последнее значение
    QList<std::function<QPair<QString, QVariant>(QPair<QString, QVariant> lastValue)>>
        buttonSettersNewValue;

    QString group = "defaultGroup";

    QJsonObject toJson() const;
    static BlockData fromJson(const QJsonObject &obj);
};

class DataContext : public QObject
{
    Q_OBJECT
public:
    explicit DataContext(QObject *parent = nullptr);

    QMap<varName, ctxVariable> variables;
    QMap<qint64, BlockData> blocksInfo;
    QMap<qint64, BlockData> dynamicsBlocksInfo;
};

#endif // DATACONTEXT_H
