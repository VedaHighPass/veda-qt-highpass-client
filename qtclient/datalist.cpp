#include "datalist.h"
#include "checkboxdelegate.h" // CheckBoxDelegate 클래스 포함
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>

DataList::DataList(QTableView *tableView, QObject *parent)
    : tableView(tableView), gridmodel(nullptr) {
}

void DataList::GridTableView() {
    QModelIndex index;
    gridmodel = new QStandardItemModel(15, 12, tableView); // 15행 12열 설정
    tableView->setModel(gridmodel);

    // 헤더 설정
    gridmodel->setHorizontalHeaderItem(0, new QStandardItem(QString(" ")));
    gridmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("사진")));
    gridmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("등록")));
    gridmodel->setHorizontalHeaderItem(3, new QStandardItem(QString("청구")));
    gridmodel->setHorizontalHeaderItem(4, new QStandardItem(QString("납부")));
    gridmodel->setHorizontalHeaderItem(5, new QStandardItem(QString("번호판")));
    gridmodel->setHorizontalHeaderItem(6, new QStandardItem(QString("출발지")));
    gridmodel->setHorizontalHeaderItem(7, new QStandardItem(QString("출발일자")));
    gridmodel->setHorizontalHeaderItem(8, new QStandardItem(QString("도착지")));
    gridmodel->setHorizontalHeaderItem(9, new QStandardItem(QString("도착일자")));
    gridmodel->setHorizontalHeaderItem(10, new QStandardItem(QString("청구일자")));
    gridmodel->setHorizontalHeaderItem(11, new QStandardItem(QString("미납요금")));

    // 데이터 초기화 및 셀 크기 설정
    for (int row = 0; row < 15; row++) {
        for (int col = 0; col < 12; col++) {
            index = gridmodel->index(row, col, QModelIndex());
            gridmodel->setData(index, "");
            tableView->setColumnWidth(col, 120);
        }
        tableView->setRowHeight(row, 50);
    }

    // 특정 열(0번 열)의 너비를 줄이기
    tableView->setColumnWidth(0, 40); // 체크박스 열 너비 설정
    tableView->setColumnWidth(1, 55);
    tableView->setColumnWidth(2, 50);
    tableView->setColumnWidth(3, 50);
    tableView->setColumnWidth(4, 50);

    for (int col = 5; col < 12; col++) {
        if (col == 6 || col == 8)
            tableView->setColumnWidth(col, 70);
        else if (col == 7 || col == 9 || col==10)
            tableView->setColumnWidth(col, 130);
        else
            tableView->setColumnWidth(col, 100); // 나머지 열 너비 설정
    }

    // CheckBoxDelegate 설정
    CheckBoxDelegate *f_checkboxdelegate = new CheckBoxDelegate(tableView);
    tableView->setItemDelegateForColumn(0, f_checkboxdelegate);
}



void DataList::populateData(const QList<QList<QVariant>> &data) {
    gridmodel->setRowCount(0); // 기존 데이터 초기화
    for (const QList<QVariant> &row : data) {
        QList<QStandardItem *> items;
        for (const QVariant &value : row) {
            items.append(new QStandardItem(value.toString()));
        }
        gridmodel->appendRow(items);
    }
}

void DataList::filterData(int column, const QString &searchText) {
    for (int row = 0; row < gridmodel->rowCount(); ++row){
        QStandardItem *item = gridmodel->item(row, column);
        if (item){
            bool matches = item->text().contains(searchText, Qt::CaseInsensitive);
            tableView -> setRowHidden(row, !matches);
        }
    }
}

void DataList::filterByDate(const QDate &startDate, const QDate &endDate){
    for (int row = 0; row < gridmodel->rowCount(); ++row) {
        QStandardItem *item = gridmodel->item(row, DataList::COL_START_DATE);
        if (item) {
            QString entryTimeStr = item->text();
            qDebug() << entryTimeStr;

            QString dateOnlyStr = entryTimeStr.left(10); // "yyyy_MM_dd" 추출
            QDate entryTime = QDate::fromString(dateOnlyStr, "yyyy-MM-dd");
            qDebug() << entryTime;
            bool matches = entryTime >= startDate && entryTime <= endDate;
            tableView->setRowHidden(row, !matches);
        }
    }
}

void DataList::addFilterCondition(const FilterCondition &condition){
    filterConditions.append(condition);
}

void DataList::applyFilters(){
    for (int row = 0; row < gridmodel->rowCount(); ++row){
        bool matches = true;
        for (const auto &condition : filterConditions){
            if (!condition(row)){
                matches = false;
                break;
            }
        }
        tableView->setRowHidden(row, !matches);
    }
}

void DataList::clearFilterConditions(){
    filterConditions.clear();
}
