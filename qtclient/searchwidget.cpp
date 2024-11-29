#include "searchwidget.h"
#include "ui_searchwidget.h"

searchWidget::searchWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::searchWidget)
{
    ui->setupUi(this);
}

searchWidget::~searchWidget()
{
    delete ui;
}
