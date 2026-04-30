#ifndef DIALOGWITHLIST_H
#define DIALOGWITHLIST_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

class DialogWithList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWithList(QWidget *parent = nullptr);

    // Метод для добавления элементов в список
    void addItems(const QStringList &items);

    // Метод для получения выбранной переменной
    QString selectedVariable() const;

    // Очистить выделение
    void clearSelection();

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *m_listWidget;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QString m_selectedVariable;

    void setupUI();
};

#endif // DIALOGWITHLIST_H