#include "mainwindow.h"


#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_stackedWidget(new QStackedWidget(this)),
    m_builder(new MenuUIBuilder(m_stackedWidget))
{
    setCentralWidget(m_stackedWidget);
    resize(500, 400);

    loadMenu("main.menucfg");
}

void MainWindow::loadMenu(const QString& filePath) {
    try {
        MenuNode root = m_parser.parseFile(filePath);

        m_builder->build(root);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}