#include <QApplication>
#include "Integration/editorscene.h"
#include "Workflow/baseworkflow.h"
#include "blockeditor.h"
#include "mydatacontext.h"

namespace MyBlocks {
class UdpServer : public BlockExecuter
{
public:
    UdpServer(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult ret;

        qDebug() << this->args[0].value();
        this->args[1] = "hello data";

        return ret;
    }
};

class ReadFile : public BlockExecuter
{
public:
    ReadFile(DataContext *context, BaseWorkFlow *workFlow)
        : BlockExecuter{context, workFlow}
    {}

    // BlockExecuter interface
public:
    ExecuteResult exec(QVariant &returnResult) override
    {
        ExecuteResult ret;
        auto path = this->args[0].value();

        qDebug() << this->args[0].value();
        QFile file(path.toString());

        if (file.open(QIODevice::ReadOnly)) {
            returnResult = file.readAll();
        }

        return ret;
    }
};

} // namespace MyBlocks

class MyWorkFlow : public BaseWorkFlow
{
public:
    MyWorkFlow(DataContext *context)
        : BaseWorkFlow{context}
    {
        int ids = 0;
        registerBlock<MyBlocks::UdpServer>(0, ids++);
        registerBlock<MyBlocks::ReadFile>(0, ids++);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MyDataContext *context = new MyDataContext;

    EditorScene *scene = new EditorScene(context);
    scene->addImportPath("qrc:/qt/qml");
    scene->setSource("qrc:/qt/qml/LogicFactory/Blocks/Main.qml");
    scene->show();
    auto watcher = scene->watcher();
    // return app.exec();

    MyWorkFlow flow(context);

    QObject::connect(scene, &EditorScene::tryExecuteChain, scene, [&flow, scene](QJsonArray chain) {
        flow.loadScript(scene->watcher()->saveScript(), false);
        flow.runChain(chain);
    });
    bool save = false;

    if (save) {
        QTimer::singleShot(20000, [scene]() {
            QFile f("temp.json");
            if (f.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(scene->watcher()->saveScript());
                f.write(doc.toJson());
            }
            f.close();
            qDebug() << "good";
        });
    } else {
        QFile f("temp.json");

        if (f.open(QIODevice::ReadOnly)) {
            auto data = QJsonDocument::fromJson(f.readAll()).object();
            scene->watcher()->loadScript(data);
            flow.loadScript(data);
        }
        f.close();

        // auto chains = flow.getChainWithType(BlockData::System, 0);
        // for (auto &i : chains) {
        //     flow.runChain(i);
        // }
    }

    return app.exec();
}
