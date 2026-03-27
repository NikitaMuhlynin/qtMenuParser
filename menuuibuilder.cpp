#include "menuuibuilder.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>

MenuUIBuilder::MenuUIBuilder(QStackedWidget* stackedWidget)
    : m_stackedWidget(stackedWidget) {}

void MenuUIBuilder::build(const MenuNode& rootNode) {
    QWidget* rootPage = buildPage(rootNode);
    m_stackedWidget->setCurrentWidget(rootPage);
}

QWidget* MenuUIBuilder::buildPage(const MenuNode& node, const QString& parentPageId) {
    QWidget* page = new QWidget();
    page->setObjectName(node.id);

    QVBoxLayout* layout = new QVBoxLayout(page);

    QLabel* titleLabel = new QLabel(node.title);
    layout->addWidget(titleLabel);

    if (!parentPageId.isEmpty()) {
        QPushButton* backButton = new QPushButton("Назад");
        QObject::connect(backButton, &QPushButton::clicked,
            [this, parentPageId]() {
                if (m_pages.contains(parentPageId)) {
                    m_stackedWidget->setCurrentWidget(m_pages[parentPageId]);
            }
        });
        layout->addWidget(backButton);
    }

    for (const MenuNode& child : node.children) {
        if (child.type == NodeType::Menu || child.type == NodeType::SubMenu) {
            QWidget* childPage = buildPage(child, node.id);
            m_pages[child.id] = childPage;

            QPushButton* menuButton = new QPushButton(child.title);
            QObject::connect(menuButton, &QPushButton::clicked,
                [this, child]() {
                    if (m_pages.contains(child.id)) {
                        m_stackedWidget->setCurrentWidget(m_pages[child.id]);
                }
            });

            layout->addWidget(menuButton);
        }
        else if (child.type == NodeType::Button) {
            QLabel* buttonLabel = new QLabel(child.title);
            layout->addWidget(buttonLabel);

            QMap<QString, QWidget*> editors;

            for (const ParameterSpec& param : child.parameters) {
                QLabel* paramLabel = new QLabel(param.name);
                layout->addWidget(paramLabel);

                QWidget* editor = createParameterEditor(param);
                editors[param.name] = editor;
                layout->addWidget(editor);
            }

            QPushButton* actionButton = new QPushButton("Выполнить");
            QObject::connect(actionButton, &QPushButton::clicked, [child, editors, this]() {
                QString result = "Action: ";

                if (child.action.has_value()) {
                    result += child.action->id + "\n";
                    result += "Command: " + child.action->command + "\n";
                } else {
                    result += "none\n";
                }

                result += "Parameters: \n";

                for (const ParameterSpec& param : child.parameters) {
                    QWidget* editor = editors[param.name];
                    QVariant value = getEditorValue(editor, param.type);
                    result += param.name + "=" + value.toString() + "\n";
                }

                QMessageBox::information(nullptr, "Button clicked", result);
            });

            layout->addWidget(actionButton);
        }
    }

    layout->addStretch();

    m_stackedWidget->addWidget(page);
    m_pages[node.id] = page;

    return page;
}

QWidget* MenuUIBuilder::createParameterEditor(const ParameterSpec& param) {
    if (param.type == "int") {
        QSpinBox* spinBox = new QSpinBox();

        if (param.minValue.isValid()) spinBox->setMinimum(param.minValue.toInt());
        if (param.maxValue.isValid()) spinBox->setMaximum(param.maxValue.toInt());
        if (param.defaultValue.isValid()) spinBox->setValue(param.defaultValue.toInt());

        return spinBox;
    }

    if (param.type == "bool") {
        QCheckBox* checkBox = new QCheckBox();
        if (param.defaultValue.isValid()) checkBox->setChecked(param.defaultValue.toBool());
        return checkBox;
    }

    QLineEdit* lineEdit = new QLineEdit();
    if (param.defaultValue.isValid()) lineEdit->setText(param.defaultValue.toString());
    return lineEdit;
}

QVariant MenuUIBuilder::getEditorValue(QWidget* editor, const QString& type) {
    if (type == "int") {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
        if (spinBox) return spinBox->value();
    }

    if (type == "bool") {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor);
        if (checkBox) return checkBox->isChecked();
    }

    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) return lineEdit->text();

    return {};
}