#ifndef HIGHPASSWINDOW_H
#define HIGHPASSWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class highPassWindow; }
QT_END_NAMESPACE

class DataList;
class DatabaseManager;

class highPassWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit highPassWindow(QWidget *parent = nullptr);
    ~highPassWindow();

private slots:
    void on_search_Button_clicked();

private:
    Ui::highPassWindow *ui;
    DataList *dataList; // DataList 객체 선언
    DatabaseManager *dbManager;
};

#endif // HIGHPASSWINDOW_H
