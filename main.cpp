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

    BlockConstructor if__("Управление", 0);
    if__.text("Если").slot("условие").text("тогда").addContainer().text("Иначе").addContainer();

    std::function<QList<QPair<QString, QVariant>>()> c1 = []() {
        QList<QPair<QString, QVariant>> ret = {{"One", 1}, {"Two", 2}, {"Three", 3}, {"Four", 4}};
        return ret;
    };

    ReporterConstructor var_("Данные", 1);
    var_.text("Значение").comboBox(c1);

    watcher->registerBlock(if__);
    watcher->registerBlock(var_);

    return app.exec();
}
