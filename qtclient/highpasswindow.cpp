#include "highpasswindow.h"
#include "ui_highpasswindow.h"
#include "datalist.h" // dataList 포함
#include "databasemanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


highPassWindow::highPassWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::highPassWindow)
    , dataList(nullptr) // 초기화
{

    ui->setupUi(this);

    // DataList 생성 및 GridTableView 호출
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->setFrameStyle(QFrame::NoFrame);

    dataList = new DataList(ui->tableView, this);
    dataList->GridTableView();

    dbManager = new DatabaseManager(this);
    connect(dbManager, &DatabaseManager::dataReady, this, [this](const QList<QList<QVariant>> &data) {
       dataList->populateData(data);
    });

    dbManager->fetchData("http://192.168.1.15:8080/records"); // 데이터 요청
}

highPassWindow::~highPassWindow() {
    delete dataList;
    delete ui;
}



void highPassWindow::on_search_Button_clicked()
{
    QString searchText = ui->search_Line->text();
    dataList->filterData(DataList::COL_PLATENUM, searchText);
    //dataList->filterByDate(ui->date_Start->date(), ui->date_End->date());
    /*QDate startDate = ui->date_Start->date();
    QDate endDate = ui->date_End->date();

    // 기존 조건 초기화
    dataList->clearFilterConditions();

    // 번호판 검색 필터 추가
    dataList->addFilterCondition([this, searchText](int row) {
        QStandardItem *item = dataList->getItem(row, DataList::COL_PLATEN UM);
        return item && item->text().contains(searchText, Qt::CaseInsensitive);
    });

    // 날짜 필터 추가
    dataList->addFilterCondition([startDate, endDate, this](int row) {
        QStandardItem *item = dataList->getItem(row, DataList::COL_START_DATE);
        if (!item) return false;

        QString dateStr = item->text().left(10); // "yyyy-MM-dd"
        QDate entryDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        return entryDate.isValid() && entryDate >= startDate && entryDate <= endDate;
    });

    // 필터 적용
    dataList->applyFilters();
*/
}

