#include <QApplication>
#include "Integration/editorscene.h"
#include "Workflow/baseworkflow.h"
#include "blockeditor.h"

class MyWorkFlow : public BaseWorkFlow
{
public:
    MyWorkFlow(DataContext *context)
        : BaseWorkFlow{context}
    {}
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    EditorScene scene;
    scene.addImportPath("qrc:/qt/qml");
    scene.setSource("qrc:/qt/qml/tttYear/Main.qml");
    scene.show();
    auto watcher = scene.watcher();
    // return app.exec();

    // DataContext context;
    // MyWorkFlow flow(&context);

    // QFile f("temp.json");

    // if (f.open(QIODevice::ReadOnly)) {
    //     flow.loadScript(QJsonDocument::fromJson(f.readAll()).object());
    // }
    // f.close();

    // auto chains = flow.getChainWithType(BlockData::System, 6);
    // for (auto &i : chains) {
    //     flow.runChain(i);
    // }

    return app.exec();
}
