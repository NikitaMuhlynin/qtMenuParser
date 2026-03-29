#ifndef ACTIONEXECUTOR_H
#define ACTIONEXECUTOR_H

#include "datamodel.h"

#include <QWidget>
#include <QStackedWidget>

class ActionExecutor {
public:
    ActionExecutor(QStackedWidget* stack);

    void execute(const ActionRequest& request);
private:
    QStackedWidget* m_stack = nullptr;
    QWidget* m_overlay = nullptr;

    void applyDisplay(int brightness, bool fullscreen);
    void showAbout();
};

#endif // ACTIONEXECUTOR_H