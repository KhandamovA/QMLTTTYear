#ifndef EDITORWATCHER_H
#define EDITORWATCHER_H

#include <QJsonValue>
#include <QObject>
#include <QQuickItem>
#include <QtQml>
#include "datacontext.h"

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
    QList<QString> slotsPlaceholders;
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

struct BlockConstructor
{
    BlockConstructor(QString group,
                     int type,
                     bool hasInput = true,
                     bool hasOutput = true,
                     QString bodyColor = "#bfcdd9",
                     QString textColor = "black");

    BlockConstructor &text(const QString &text);
    BlockConstructor &slot(const QString &placeholder);
    BlockConstructor &addContainer();
    BlockConstructor &comboBox(
        std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant> lastValue)>
            callCurrentList);
    BlockConstructor &button(
        std::function<QPair<QString, QVariant>(QPair<QString, QVariant> lastValue)>
            callSetterNewValue);

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
    ReporterConstructor &slot(const QString &placeholder);
    ReporterConstructor &comboBox(
        std::function<QList<QPair<QString, QVariant>>(QPair<QString, QVariant> lastValue)>
            callCurrentList);
    ReporterConstructor &button(
        std::function<QPair<QString, QVariant>(QPair<QString, QVariant> lastValue)>
            callSetterNewValue);

    operator BlockData() { return data; }

private:
    BlockData data;
    int currentRow = 0;
};

class EditorWatcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    friend class BlockEditor;

public:
    EditorWatcher(QObject *parent = nullptr);

    void registerBlock(BlockData data);

public slots:
    void handleResponse(QJsonValue response);
    QJsonValue qml_query(const QString &method, QJsonValue data);

signals:
    void qml_signal(const QString &method, QJsonValue data);

private:
    int getUniqDynamicBlockType(int id);
    QJsonValue sendCommand(const QString &method, QJsonValue data);

    QMap<qint64, BlockData> m_blocksInfo;
    QMap<qint64, BlockData> m_DynamicsBlocksInfo;
    QList<QJsonValue> m_responses;
    DataContext m_dataContext;

private:
    void createNewBlock();
    void deleteVariable();
    bool saveToFile(QJsonValue data);
    QJsonValue loadFromFile(const QString &pathToFile);
    QString createNewVar(const QString &oldName = "");

    QJsonArray comboBoxListVariablesNames() const;
};

#endif // EDITORWATCHER_H
