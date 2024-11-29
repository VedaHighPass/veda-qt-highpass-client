#ifndef DATALIST_H
#define DATALIST_H

#include <QWidget>
#include <QTableWidget>
#include <QTableView>
#include <QStandardItemModel>

using FilterCondition = std::function<bool(int row)>;

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
        COL_REGISTRATION,
        COL_BILL,
        COL_PAYMENT,
        COL_PLATENUM,
        COL_START_LOCATION,
        COL_START_DATE,
        COL_END_LOCATION,
        COL_END_DATE,
        COL_BILL_DATE,
        COL_UNPAIDFEE,
        COL_COUNT // 총 열개수
    };

    void addFilterCondition(const FilterCondition &condition);
    void applyFilters();
    void clearFilterConditions();

public slots:
    void filterData(int column, const QString &searchText);
    void filterByDate(const QDate &startDate, const QDate &endDate);

private:
    QTableView *tableView;
    QStandardItemModel *gridmodel;
    QVector<FilterCondition> filterConditions;
};

#endif // DATALIST_H
