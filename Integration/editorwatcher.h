#ifndef EDITORWATCHER_H
#define EDITORWATCHER_H

#include <QJsonValue>
#include <QObject>
#include <QQuickItem>
#include <QtQml>

struct BlockData
{
    int type = 0;
    QList<QString> viewTexts;
    bool hasInput = true;
    bool hasOutput = true;
    QString textColor = "black";
    QString bodyColor = "#bfcdd9";
    int blockShape = 0; ///< 0 - обычный блок, 1 - репортер

    QString group = "defaultGroup";

    QJsonObject toJson() const;
    static BlockData fromJson(const QJsonObject &obj);
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
signals:
    void qml_signal(const QString &method, QJsonValue data);

private:
    QJsonValue sendCommand(const QString &method, QJsonValue data);

    QList<QJsonValue> m_responses;
};

#endif // EDITORWATCHER_H
