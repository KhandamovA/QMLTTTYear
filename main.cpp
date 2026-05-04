#include <QApplication>
#include "Integration/editorscene.h"
#include "blockeditor.h"

QList<BlockData> standartKitBlocks(int startTypeWith = 0)
{
    QList<BlockData> ret;

    return ret;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    EditorScene scene;
    scene.addImportPath("qrc:/qt/qml");
    scene.setSource("qrc:/qt/qml/tttYear/Main.qml");
    // scene.setSource("qrc:/qt/qml/tttYear/UIElements/ConstructorScene.qml");
    scene.show();

    auto watcher = scene.watcher();

    for (auto &i : standartKitBlocks()) {
        watcher->registerBlock(i);
    }

    // BlockEditor editor;
    // editor.show();

    return app.exec();
}
