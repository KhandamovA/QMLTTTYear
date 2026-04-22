#include <QApplication>
#include "Integration/editorscene.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    EditorScene scene;
    scene.addImportPath("qrc:/qt/qml");
    scene.setSource("qrc:/qt/qml/tttYear/Main.qml");
    scene.show();

    auto watcher = scene.watcher();

    BlockData if_;
    if_.type = 0;
    if_.viewTexts = {"Если $$ тогда", "Иначе", ""};
    if_.group = "Управление";
    watcher->registerBlock(if_);

    BlockData test;
    test.type = 1;
    test.viewTexts = {"Вернуть $$"};
    test.blockShape = 1;
    test.group = "Репортеры";
    test.bodyColor = "cyan";
    watcher->registerBlock(test);

    BlockData while_;
    while_.type = 2;
    while_.viewTexts = {"Повторять пока $$", ""};
    while_.group = "Управление";
    watcher->registerBlock(while_);

    return app.exec();
}
