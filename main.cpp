#include <QApplication>
#include "Integration/editorscene.h"
#include "blockeditor.h"

QList<BlockData> standartKitBlocks(int startTypeWith = 0)
{
    QList<BlockData> ret;

    BlockConstructor while_("Управление", startTypeWith + 0);
    while_.text("Повторять пока").slot("true").addContainer();
    ret.append(while_);

    BlockConstructor while_2("Управление", startTypeWith + 1);
    while_2.text("Повторить").slot("count").text("раз").addContainer();
    ret.append(while_2);

    BlockConstructor if_1("Управление", startTypeWith + 2);
    if_1.text("Если").slot("true").text("тогда").addContainer().text("Иначе").addContainer();
    ret.append(if_1);

    BlockConstructor if_2("Управление", startTypeWith + 3);
    if_2.text("Если").slot("true").text("тогда").addContainer();
    ret.append(if_2);

    BlockConstructor debug_("Отладка", startTypeWith + 4);
    debug_.text("Вывод в консоль").slot("text");
    ret.append(debug_);

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
