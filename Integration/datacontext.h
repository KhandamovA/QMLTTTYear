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
        uno = other;
        return uno;
    }
    operator QVariant() { return uno; }

    // Методы работы с листом
    void append(const QVariant &item) { list.append(item); }
    QVariant operator[](int index) { return list[index]; }
    int count() const { return list.count(); }
    int indexOf(const QVariant &item) { return list.indexOf(item); }

    // Методы работы со словарем
    void setValue(const QString &key, const QVariant &value) { map[key] = value; }
    QVariant value(const QString &key)
    {
        auto find = map.find(key);
        if (find != map.end()) {
            return *find;
        }
        return QVariant();
    }
    QStringList keys() const { return map.keys(); }
    void removeKey(const QString &key) { map.remove(key); }

    // Метод для проверки типа (не самый надежный)
    int typeId()
    {
        return map.count() ? QMetaType::Type::QVariantMap
                           : (list.count() ? QMetaType::Type::QVariantList : uno.typeId());
    }

    // Метод очистки для всего
    void clear()
    {
        list.clear();
        map.clear();
        uno = QVariant();
    }

private:
    QVariant uno;
    QList<QVariant> list;
    QMap<QString, QVariant> map;
};

struct slotInfo
{
    enum SlotType { Plain = 0, ComboBox = 1, Button = 2 };

    SlotType type = Plain;
    QString name;

    std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant>)> getterList;
    std::function<QPair<QString, QVariant>(QPair<QString, QVariant> lastValue)> getterButtonValue;
};

struct BlockData
{
    enum Shape { Block = 0, Reporter = 1 };
    enum Origin { Custom = 0, Dynamic = 1, System = 2 };

    int type = -1;
    int origin = 0;
    QList<QString> viewTexts;
    bool hasInput = true;
    bool hasOutput = true;
    QString textColor = "black";
    QString bodyColor = "#bfcdd9";
    int blockShape = 0; ///< 0 - обычный блок, 1 - репортер
    QJsonObject tags;

    QList<slotInfo> slotsInfo;

    QString group = "defaultGroup";

    QJsonObject toJson() const;
    static BlockData fromJson(const QJsonObject &obj);
};

struct BlockConstructor
{
    BlockConstructor(QString group,
                     int type,
                     bool hasInput = true,
                     bool hasOutput = true,
                     QString bodyColor = "#bfcdd9",
                     QString textColor = "black");

    BlockConstructor &text(const QString &text);
    BlockConstructor &addContainer();

    BlockConstructor &slot(const QString &name);
    BlockConstructor &comboBox(
        std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant> lastValue)>
            callCurrentList,
        const QString &name);
    BlockConstructor &button(
        std::function<QPair<QString, QVariant>(QPair<QString, QVariant> lastValue)>
            callSetterNewValue,
        const QString &name);

    operator BlockData() { return data; }

private:
    BlockData data;
    int currentRow = 0;
};

struct ReporterConstructor
{
    ReporterConstructor(QString group,
                        int type,
                        QString bodyColor = "#bfcdd9",
                        QString textColor = "black");

    ReporterConstructor &text(const QString &text);

    ReporterConstructor &slot(const QString &name);
    ReporterConstructor &comboBox(
        std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant> lastValue)>
            callCurrentList,
        const QString &name);
    ReporterConstructor &button(
        std::function<QPair<QString, QVariant>(QPair<QString, QVariant> lastValue)>
            callSetterNewValue,
        const QString &name);

    operator BlockData() { return data; }

private:
    BlockData data;
    int currentRow = 0;
};

class DataContext : public QObject
{
    Q_OBJECT
public:
    explicit DataContext(QObject *parent = nullptr);

    // Переменные, массивы, словари
    QMap<varName, ctxVariable> variables;
    // Блоки которые создаются после компиляции origin = 0, ключ = тип
    QMap<qint64, BlockData> blocksInfo;
    // Блоки которые создаются динамически во время редактирования кода origin = 1, ключ = тип
    QMap<qint64, BlockData> dynamicsBlocksInfo;
    // Блоки которые существуют всегда origin = 2, ключ = тип, к ним относятся блоки для работы с данными
    QMap<qint64, BlockData> systemBlocksInfo;

    BlockData getBlockInfo(int origin, int type);
    QList<BlockData> allBlocks() const;

private:
    void addStandartBlocks();
};

#endif // DATACONTEXT_H
