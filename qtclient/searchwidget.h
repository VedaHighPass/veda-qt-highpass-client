#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

namespace Ui {
class searchWidget;
}

class searchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit searchWidget(QWidget *parent = nullptr);
    ~searchWidget();

private:
    Ui::searchWidget *ui;
};

#endif // SEARCHWIDGET_H
