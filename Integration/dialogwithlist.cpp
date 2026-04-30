#include "dialogwithlist.h"

DialogWithList::DialogWithList(QWidget *parent)
    : QDialog(parent)
    , m_listWidget(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_selectedVariable("")
{
    setupUI();
}

void DialogWithList::setupUI()
{
    setWindowTitle("Удаление переменной");
    setMinimumSize(400, 300);

    // Создаем главный вертикальный layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Создаем надпись сверху
    QLabel *label = new QLabel("Выберите переменную, которую хотите удалить:", this);
    label->setAlignment(Qt::AlignCenter);

    // Создаем QListWidget
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Создаем горизонтальный layout для кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // Создаем кнопки
    m_okButton = new QPushButton("ОК", this);
    m_cancelButton = new QPushButton("Отмена", this);

    // Добавляем кнопки в layout
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();

    // Добавляем все элементы в главный layout
    mainLayout->addWidget(label);
    mainLayout->addWidget(m_listWidget);
    mainLayout->addLayout(buttonLayout);

    // Подключаем сигналы кнопок
    connect(m_okButton, &QPushButton::clicked, this, &DialogWithList::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &DialogWithList::onCancelClicked);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &DialogWithList::onDoubleClicked);

    // Устанавливаем layout
    setLayout(mainLayout);
}

void DialogWithList::addItems(const QStringList &items)
{
    if (m_listWidget) {
        m_listWidget->clear();
        for (auto &i : items) {
            QListWidgetItem *item = new QListWidgetItem(i);
            item->setSizeHint({32, 32});
            m_listWidget->addItem(item);
        }
    }
}

QString DialogWithList::selectedVariable() const
{
    return m_selectedVariable;
}

void DialogWithList::clearSelection()
{
    if (m_listWidget) {
        m_listWidget->clearSelection();
    }
    m_selectedVariable = "";
}

void DialogWithList::onOkClicked()
{
    // Проверяем, выделен ли элемент
    QListWidgetItem *currentItem = m_listWidget->currentItem();

    if (!currentItem) {
        // Если ничего не выделено, показываем уведомление
        QMessageBox::warning(this,
                             "Предупреждение",
                             "Ничего не выбрано!\nПожалуйста, выберите переменную для удаления.",
                             QMessageBox::Ok);
        return; // Не закрываем окно
    }

    // Сохраняем выбранную переменную
    m_selectedVariable = currentItem->text();

    // Закрываем диалог с результатом Accepted
    accept();
}

void DialogWithList::onCancelClicked()
{
    m_selectedVariable = "";
    reject(); // Закрываем диалог с результатом Rejected
}

void DialogWithList::onDoubleClicked(QListWidgetItem *item)
{
    m_selectedVariable = item->text();
    accept();
}