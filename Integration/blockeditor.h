#ifndef BLOCKEDITOR_H
#define BLOCKEDITOR_H

#include "editorwatcher.h"

#include <QComboBox>
#include <QDialog>
#include <QTableWidget>

class EditorScene;

class BlockEditor : public QDialog
{
    Q_OBJECT
public:
    BlockEditor(EditorWatcher *parent);

    BlockData save() const;
    void load(const BlockData &blockData);

    void updateBlock();
    bool isAccepted() const;

private slots:
    void onAddText();
    void onAddSlot();
    void onRemoveLast();
    void onClearAll();
    void onTypeChanged(int index);
    void onBodyColorChanged();
    void onTextColorChanged();
    void onTableCellChanged(int row, int column);
    void rebuildPreview();
    void onSave();
    void onCancel();

private:
    struct Piece
    {
        enum Type { Text, Slot };
        Type type;
        QString text;        // для текста
        QString placeholder; // для слота
    };

    void updatePieceTable();
    void updateBlockDataFromUI();
    void loadPiecesFromBlockData(const BlockData &blockData);

    EditorWatcher *watcher;
    EditorScene *scene;
    BlockData data;
    bool mIsAccepted = false;

    QTableWidget *pieceTable;
    QComboBox *blockTypeCombo;
    QPushButton *bodyColorBtn;
    QPushButton *textColorBtn;
    QList<Piece> pieces;
};

#endif // BLOCKEDITOR_H