#ifndef HIGHPASSWINDOW_H
#define HIGHPASSWINDOW_H

#include <QMainWindow>
#include <QUrlQuery>


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

    void on_mail_Button_clicked();
    void initializeDatabaseManager();

private:
    Ui::highPassWindow *ui;
    DataList *dataList; // DataList 객체 선언
    DatabaseManager *dbManager;
    bool validateIpAddress(const QString &ipAddress) const;

    int currentPage = 1; // 현재 페이지
    int totalPages = 1;  // 총 페이지 수
    const int pageSize = 10; // 한 페이지당 항목 수

    void updatePageButtons(int totalPages);
    QUrlQuery currentQueryParams;
    void updateIcon();
};

#endif // HIGHPASSWINDOW_H
