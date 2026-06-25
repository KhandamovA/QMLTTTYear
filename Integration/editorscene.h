#ifndef EDITORSCENE_H
#define EDITORSCENE_H

#include "editorwatcher.h"

#include <QWidget>

class QQuickView;
class EditorScene : public QWidget
{
    Q_OBJECT
public:
    explicit EditorScene(DataContext *context, QWidget *parent = nullptr);

    void setSource(const QString &src);
    void addImportPath(const QString &impPath);

    EditorWatcher *watcher() const;
    DataContext *context() const;

    QJsonObject saveScript();
    void loadScript(QJsonObject data);

signals:
    void tryExecuteChain(QJsonArray chain);

private:
    EditorWatcher *m_watcher;
    QQuickView *view;
};

#endif // EDITORSCENE_H
