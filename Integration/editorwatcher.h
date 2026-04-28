#ifndef EDITORWATCHER_H
#define EDITORWATCHER_H

#include <QJsonValue>
#include <QObject>
#include <QQuickItem>
#include <QtQml>

struct BlockData
{
    enum Shape { Block = 0, Reporter = 1 };

    int type = 0;
    QList<QString> viewTexts;
    bool hasInput = true;
    bool hasOutput = true;
    QString textColor = "black";
    QString bodyColor = "#bfcdd9";
    int blockShape = 0; ///< 0 - обычный блок, 1 - репортер

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

public:
    EditorWatcher(QObject *parent = nullptr);

    void registerBlock(BlockData data);

public slots:
    void handleResponse(QJsonValue response);
    QJsonValue qml_query(const QString &method, QJsonValue data);

signals:
    void qml_signal(const QString &method, QJsonValue data);

private:
    QJsonValue sendCommand(const QString &method, QJsonValue data);

    QMap<qint64, BlockData> m_blocksInfo;
    QList<QJsonValue> m_responses;
};

#endif // EDITORWATCHER_H
