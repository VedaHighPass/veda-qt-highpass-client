#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datalist.h"
#include "highpasswindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    highPassWindow* highPass = new highPassWindow;
    setCentralWidget(highPass);
}

MainWindow::~MainWindow()
{
    delete ui;
}
