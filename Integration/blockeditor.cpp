#include "blockeditor.h"
#include "editorscene.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

BlockEditor::BlockEditor(QWidget *parent)
    : QDialog(parent)
{
    resize(1000, 700);

    QGridLayout *mainLayout = new QGridLayout(this);

    // Левая колонка - таблица кусочков (0, 0)
    QGroupBox *piecesGroup = new QGroupBox("Block Pieces");
    QVBoxLayout *piecesLayout = new QVBoxLayout(piecesGroup);

    pieceTable = new QTableWidget();
    pieceTable->setColumnCount(2);
    pieceTable->setHorizontalHeaderLabels({"Type", "Value"});
    pieceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    pieceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    piecesLayout->addWidget(pieceTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addTextBtn = new QPushButton("Add Text");
    QPushButton *addSlotBtn = new QPushButton("Add Slot");
    QPushButton *removeLastBtn = new QPushButton("Remove Last");
    QPushButton *clearAllBtn = new QPushButton("Clear All");

    connect(addTextBtn, &QPushButton::clicked, this, &BlockEditor::onAddText);
    connect(addSlotBtn, &QPushButton::clicked, this, &BlockEditor::onAddSlot);
    connect(removeLastBtn, &QPushButton::clicked, this, &BlockEditor::onRemoveLast);
    connect(clearAllBtn, &QPushButton::clicked, this, &BlockEditor::onClearAll);

    buttonLayout->addWidget(addTextBtn);
    buttonLayout->addWidget(addSlotBtn);
    buttonLayout->addWidget(removeLastBtn);
    buttonLayout->addWidget(clearAllBtn);
    piecesLayout->addLayout(buttonLayout);

    mainLayout->addWidget(piecesGroup, 0, 0, 2, 1);

    // Правая верхняя часть - сцена (0, 1)
    scene = new EditorScene;
    scene->addImportPath("qrc:/qt/qml");
    scene->setSource("qrc:/qt/qml/tttYear/UIElements/ConstructorScene.qml");
    scene->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(scene, 0, 1, 1, 1);
    mainLayout->setRowStretch(0, 1);

    // Правая нижняя часть - настройки (1, 1)
    QGroupBox *settingsGroup = new QGroupBox("Block Settings");
    QGridLayout *settingsLayout = new QGridLayout(settingsGroup);

    settingsLayout->addWidget(new QLabel("Block Type:"), 0, 0);
    blockTypeCombo = new QComboBox();
    blockTypeCombo->addItem("Regular Block", BlockData::Block);
    blockTypeCombo->addItem("Reporter", BlockData::Reporter);
    connect(blockTypeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &BlockEditor::onTypeChanged);
    settingsLayout->addWidget(blockTypeCombo, 0, 1);

    settingsLayout->addWidget(new QLabel("Body Color:"), 1, 0);
    bodyColorBtn = new QPushButton();
    bodyColorBtn->setFixedSize(50, 25);
    connect(bodyColorBtn, &QPushButton::clicked, this, &BlockEditor::onBodyColorChanged);
    settingsLayout->addWidget(bodyColorBtn, 1, 1);

    settingsLayout->addWidget(new QLabel("Text Color:"), 2, 0);
    textColorBtn = new QPushButton();
    textColorBtn->setFixedSize(50, 25);
    connect(textColorBtn, &QPushButton::clicked, this, &BlockEditor::onTextColorChanged);
    settingsLayout->addWidget(textColorBtn, 2, 1);

    mainLayout->addWidget(settingsGroup, 1, 1, 1, 1);

    // Кнопки Save/Cancel внизу
    QHBoxLayout *dialogButtonsLayout = new QHBoxLayout();
    dialogButtonsLayout->addStretch();

    QPushButton *saveBtn = new QPushButton("Save");
    QPushButton *cancelBtn = new QPushButton("Cancel");

    connect(saveBtn, &QPushButton::clicked, this, &BlockEditor::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &BlockEditor::onCancel);

    dialogButtonsLayout->addWidget(saveBtn);
    dialogButtonsLayout->addWidget(cancelBtn);

    mainLayout->addLayout(dialogButtonsLayout, 2, 0, 1, 2);

    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 1);

    connect(pieceTable, &QTableWidget::cellChanged, this, &BlockEditor::onTableCellChanged);
}

void BlockEditor::onSave()
{
    accept(); // Закрываем диалог с кодом Accepted
}

void BlockEditor::onCancel()
{
    reject(); // Закрываем диалог с кодом Rejected
}

BlockData BlockEditor::save() const
{
    BlockData result = data;

    // Собираем viewTexts и slotsPlaceholders из pieces
    result.viewTexts.clear();
    result.slotsPlaceholders.clear();

    QString currentRow;

    for (const Piece &p : pieces) {
        if (p.type == Piece::Text) {
            currentRow += p.text;
        } else if (p.type == Piece::Slot) {
            currentRow += " $$ ";
            result.slotsPlaceholders.append(p.placeholder);
        }
    }

    if (!currentRow.isEmpty()) {
        result.viewTexts.append(currentRow);
    } else {
        result.viewTexts.append("");
    }

    return result;
}

void BlockEditor::load(const BlockData &blockData)
{
    data = blockData;

    // Загружаем настройки в UI
    int index = blockTypeCombo->findData(blockData.blockShape);
    if (index >= 0) {
        blockTypeCombo->setCurrentIndex(index);
    }

    bodyColorBtn->setStyleSheet(QString("background-color: %1").arg(blockData.bodyColor));
    textColorBtn->setStyleSheet(
        QString("color: %1; background-color: white").arg(blockData.textColor));

    // Загружаем pieces из BlockData
    loadPiecesFromBlockData(blockData);
    updatePieceTable();

    // Обновляем превью
    rebuildPreview();
}

void BlockEditor::loadPiecesFromBlockData(const BlockData &blockData)
{
    pieces.clear();

    if (blockData.viewTexts.isEmpty()) {
        return;
    }

    // Берем первую строку (у вас обычно одна строка)
    QString viewText = blockData.viewTexts.first();

    // Парсим viewText
    QStringList parts = viewText.split(" $$ ", Qt::KeepEmptyParts);

    int slotIndex = 0;

    for (int i = 0; i < parts.size(); ++i) {
        if (!parts[i].isEmpty()) {
            // Добавляем текст
            Piece textPiece;
            textPiece.type = Piece::Text;
            textPiece.text = parts[i];
            pieces.append(textPiece);
        }

        // Добавляем слот если это не последний элемент
        if (i < parts.size() - 1) {
            Piece slotPiece;
            slotPiece.type = Piece::Slot;
            if (slotIndex < blockData.slotsPlaceholders.size()) {
                slotPiece.placeholder = blockData.slotsPlaceholders[slotIndex];
            } else {
                slotPiece.placeholder = "slot";
            }
            pieces.append(slotPiece);
            slotIndex++;
        }
    }

    // Если строка начинается со слота
    if (viewText.startsWith(" $$ ")) {
        Piece slotPiece;
        slotPiece.type = Piece::Slot;
        slotPiece.placeholder = slotIndex < blockData.slotsPlaceholders.size()
                                    ? blockData.slotsPlaceholders[slotIndex]
                                    : "slot";
        pieces.prepend(slotPiece);
    }

    // Если строка заканчивается слотом
    if (viewText.endsWith(" $$ ")) {
        Piece slotPiece;
        slotPiece.type = Piece::Slot;
        slotPiece.placeholder = slotIndex < blockData.slotsPlaceholders.size()
                                    ? blockData.slotsPlaceholders[slotIndex]
                                    : "slot";
        pieces.append(slotPiece);
    }
}

void BlockEditor::onTableCellChanged(int row, int column)
{
    if (column == 1 && row < pieces.size()) {
        Piece &p = pieces[row];
        QString newValue = pieceTable->item(row, 1)->text();

        if (p.type == Piece::Text) {
            p.text = newValue;
        } else if (p.type == Piece::Slot) {
            p.placeholder = newValue;
        }
        rebuildPreview();
    }
}

void BlockEditor::onAddText()
{
    Piece p;
    p.type = Piece::Text;
    p.text = "new text";
    pieces.append(p);
    updatePieceTable();
    rebuildPreview();
}

void BlockEditor::onAddSlot()
{
    Piece p;
    p.type = Piece::Slot;
    p.placeholder = "slot";
    pieces.append(p);
    updatePieceTable();
    rebuildPreview();
}

void BlockEditor::onRemoveLast()
{
    if (!pieces.isEmpty()) {
        pieces.removeLast();
        updatePieceTable();
        rebuildPreview();
    }
}

void BlockEditor::onClearAll()
{
    pieces.clear();
    updatePieceTable();
    rebuildPreview();
}

void BlockEditor::onTypeChanged(int index)
{
    Q_UNUSED(index);
    data.blockShape = blockTypeCombo->currentData().toInt();
    rebuildPreview();
}

void BlockEditor::onBodyColorChanged()
{
    QColor color = QColorDialog::getColor(QColor(data.bodyColor), this);
    if (color.isValid()) {
        data.bodyColor = color.name();
        bodyColorBtn->setStyleSheet(QString("background-color: %1").arg(data.bodyColor));
        rebuildPreview();
    }
}

void BlockEditor::onTextColorChanged()
{
    QColor color = QColorDialog::getColor(QColor(data.textColor), this);
    if (color.isValid()) {
        data.textColor = color.name();
        textColorBtn->setStyleSheet(
            QString("color: %1; background-color: white").arg(data.textColor));
        rebuildPreview();
    }
}

void BlockEditor::updatePieceTable()
{
    disconnect(pieceTable, &QTableWidget::cellChanged, this, &BlockEditor::onTableCellChanged);

    pieceTable->setRowCount(pieces.size());

    for (int i = 0; i < pieces.size(); ++i) {
        const Piece &p = pieces[i];

        QTableWidgetItem *typeItem = new QTableWidgetItem();
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        typeItem->setText(p.type == Piece::Text ? "Text" : "Slot");
        pieceTable->setItem(i, 0, typeItem);

        QTableWidgetItem *valueItem = new QTableWidgetItem();
        valueItem->setText(p.type == Piece::Text ? p.text : p.placeholder);
        pieceTable->setItem(i, 1, valueItem);
    }

    connect(pieceTable, &QTableWidget::cellChanged, this, &BlockEditor::onTableCellChanged);
}

void BlockEditor::rebuildPreview()
{
    updateBlockDataFromUI();
    updateBlock();
}

void BlockEditor::updateBlockDataFromUI()
{
    data.viewTexts.clear();
    data.slotsPlaceholders.clear();
    data.comboBoxCallCurrentList.clear();

    QString currentRow;

    for (const Piece &p : pieces) {
        if (p.type == Piece::Text) {
            currentRow += p.text;
        } else if (p.type == Piece::Slot) {
            currentRow += " $$ ";
            data.slotsPlaceholders.append(p.placeholder);
        }
    }

    if (!currentRow.isEmpty()) {
        data.viewTexts.append(currentRow);
    } else {
        data.viewTexts.append("");
    }
}

void BlockEditor::updateBlock()
{
    auto watcher = scene->watcher();
    watcher->sendCommand("setConstructorBlock", data.toJson());
}