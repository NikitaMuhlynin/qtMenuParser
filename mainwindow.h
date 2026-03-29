#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include "imenuparser.h"
#include "menuuibuilder.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget* m_stackedWidget;
    IMenuParser m_parser;
    MenuUIBuilder m_builder;

    void loadMenu(const QString& filePath);
};

#endif // MAINWINDOW_H
