#ifndef DATALIST_H
#define DATALIST_H

#include <QWidget>
#include <QTableWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QNetworkAccessManager>

class DataList : public QWidget
{
    Q_OBJECT

public:
    explicit DataList(QTableView *tableView, QObject *parent = nullptr);
    void GridTableView(); // 테이블 뷰 초기화 메서드
    void populateData(const QList<QList<QVariant>> &data);

    enum Columns {
        COL_CHECKBOX,
        COL_PHOTO,
        COL_PATH,
        COL_PLATENUM,
        COL_START_LOCATION,
        COL_START_DATE,
        COL_END_LOCATION,
        COL_END_DATE,
        COL_BILL_DATE,
        COL_UNPAIDFEE,
        COL_REGISTRATION,
        COL_PAYMENT,
        COL_COUNT // 총 열개수
    };
    QList<QPair<QString, QString>> getCheckedClients() const;
public slots :
    void onImageDownloaded(); // 이미지 다운로드 슬롯

private:
    QTableView *tableView;
    QStandardItemModel *gridmodel;
    QNetworkAccessManager *networkManager;
    void updateRowColors();
};

#endif // DATALIST_H
