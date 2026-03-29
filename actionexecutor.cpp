#include "actionexecutor.h"

#include <QMessageBox>

ActionExecutor::ActionExecutor(QStackedWidget* stack)
    : m_stack(stack) {}

void ActionExecutor::execute(const ActionRequest& request) {
    if (request.actionId == "apply_display") {
        int brightness = request.parameters["Brightness"].toInt();
        bool fullscreen = request.parameters["Fullscreen"].toBool();

        applyDisplay(brightness, fullscreen);
    } else if (request.actionId == "show_about") {
        showAbout();
    } else {
        QMessageBox::information(nullptr, "Button clicked", "Unknown action");
    }
}

void ActionExecutor::applyDisplay(int brightness, bool fullscreen) {
    QWidget *window = m_stack->window();
    if (window) {
        if (fullscreen) {
            if (!window->isFullScreen())
                window->showFullScreen();
        } else {
            if (window->isFullScreen())
                window->showNormal();
        }
    }

    if (!m_overlay) {
        m_overlay = new QWidget(m_stack);
        m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_overlay->setAttribute(Qt::WA_StyledBackground, true);
        m_overlay->hide();
    }

    m_overlay->setGeometry(m_stack->rect());
    m_overlay->raise();

    int alpha = (100 - brightness) * 255 / 100;

    if (alpha <= 0) {
        m_overlay->hide();
    } else {
        m_overlay->setStyleSheet(
            QString("background-color: rgba(0, 0, 0, %1);").arg(alpha)
            );
        m_overlay->show();
    }
}

void ActionExecutor::showAbout() {
    QMessageBox::information(nullptr, "Button clicked", "About this program");
}