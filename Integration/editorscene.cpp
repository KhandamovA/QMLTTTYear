#include "editorscene.h"

#include <QGridLayout>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>

EditorScene::EditorScene(QWidget *parent)
    : QWidget{parent}
{
    m_watcher = new EditorWatcher;

    view = new QQuickView();

    view->setResizeMode(QQuickView::SizeRootObjectToView);

    QGridLayout *l = new QGridLayout;
    setLayout(l);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(QWidget::createWindowContainer(view, this));

    resize(600, 400);

    addImportPath("qrc:/qt/qml");
    setSource("qrc:/qt/qml/LogicFactory/Blocks/Main.qml");
}

void EditorScene::setSource(const QString &src)
{
    view->setSource(QUrl(src));

    //Установка после подгрузки сорсов
    QQuickItem *rootObject = view->rootObject();
    if (rootObject) {
        rootObject->setProperty("watcher", QVariant::fromValue(m_watcher));
        m_watcher->init();
    }
}

void EditorScene::addImportPath(const QString &impPath)
{
    view->engine()->addImportPath(impPath);
}

EditorWatcher *EditorScene::watcher() const
{
    return m_watcher;
}

DataContext *EditorScene::context() const
{
    return m_watcher->dataContext();
}
