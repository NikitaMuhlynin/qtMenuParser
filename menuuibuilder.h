#ifndef MENUUIBUILDER_H
#define MENUUIBUILDER_H

#include "datamodel.h"
#include "actionexecutor.h"

#include <QWidget>
#include <QStackedWidget>
#include <QMap>

class MenuUIBuilder
{
public:
    explicit MenuUIBuilder(QStackedWidget* stackedWidget);

    void build(const MenuNode& rootNode);

private:
    QStackedWidget* m_stackedWidget;
    QMap<QString, QWidget*> m_pages;
    ActionExecutor executor;

    QWidget* buildPage(const MenuNode& node, const QString& parentPageId = QString());
    QWidget* createParameterEditor(const ParameterSpec& param);
    QVariant getEditorValue(QWidget* editor, const QString& type);
};

#endif // MENUUIBUILDER_H
