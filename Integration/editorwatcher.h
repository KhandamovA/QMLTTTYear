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

class QmlMessagePack : public QObject
{
    Q_OBJECT
public:
    QJsonValue exec();
    qint64 id = 0;

public slots:
    void messageGet(qint64 id, QJsonValue value);

private:
    QJsonValue returnValue;
    bool stop = false;
};

class EditorWatcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    friend class BlockEditor;

public:
    EditorWatcher(QObject *parent = nullptr);

    ///
    /// \brief registerBlock Регистрация блока
    /// \param data Данные блока
    /// \param checkDefine Если это динамический блок, включать ли
    /// проверку на существование определителя блока
    /// (этот параметр требуется ставить false если загружается сохраненный
    /// скрипт во избежание ошибок от двойного создания определителя)
    ///
    void registerBlock(BlockData data, bool checkDefine = true);

public slots:

    QJsonObject saveScript();
    void loadScript(QJsonObject data);
    void clear();

    void handleResponse(QJsonValue response, qint64 signalId);
    QJsonValue qml_query(const QString &method, QJsonValue data);

signals:
    void messageGet(qint64 signalId, QJsonValue value);

    void qml_signal(const QString &method, QJsonValue data, qint64 signalId);

private:
    int getUniqDynamicBlockType(int id);
    QJsonValue sendCommand(const QString &method, QJsonValue data);

    QMap<qint64, BlockData> m_blocksInfo;
    QMap<qint64, BlockData> m_DynamicsBlocksInfo;

    DataContext m_dataContext;
    qint64 signalId = 0;

private:
    void createNewBlock();
    void deleteVariable();
    bool saveToFile(QJsonValue data);
    QJsonValue loadFromFile(const QString &pathToFile);
    QString createNewVar(const QString &oldName = "");

    QJsonArray comboBoxListVariablesNames() const;
};

#endif // EDITORWATCHER_H
