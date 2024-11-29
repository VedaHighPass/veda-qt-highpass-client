#include "highpasswindow.h"
#include "ui_highpasswindow.h"
#include "datalist.h" // dataList 포함
#include "databasemanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QButtonGroup>

highPassWindow::highPassWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::highPassWindow)
    , dataList(nullptr) // 초기화
{
    ui->setupUi(this);

    QButtonGroup *entrygroup = new QButtonGroup(this);
    entrygroup->addButton(ui->entry_ALL);
    entrygroup->addButton(ui->entry_SEOUL);
    entrygroup->addButton(ui->entry_DAEGU);
    entrygroup->addButton(ui->entry_DAEJUN);
    entrygroup->addButton(ui->entry_BUSAN);

    entrygroup->setExclusive(false);

    connect(ui->entry_ALL, &QRadioButton::clicked, this, [this]() {
        if (ui->entry_ALL->isChecked()) {
            ui->entry_DAEGU->setEnabled(false);
            ui->entry_DAEJUN->setEnabled(false);
            ui->entry_SEOUL->setEnabled(false);
            ui->entry_BUSAN->setEnabled(false);
        } else {
            ui->entry_DAEGU->setEnabled(true);
            ui->entry_DAEJUN->setEnabled(true);
            ui->entry_SEOUL->setEnabled(true);
            ui->entry_BUSAN->setEnabled(true);
        }
    });

    QButtonGroup *exitgroup = new QButtonGroup(this);
    exitgroup->addButton(ui->exit_ALL);
    exitgroup->addButton(ui->exit_SEOUL);
    exitgroup->addButton(ui->exit_DAEGU);
    exitgroup->addButton(ui->exit_DAEJUN);
    exitgroup->addButton(ui->exit_BUSAN);

    exitgroup->setExclusive(false);

    connect(ui->exit_ALL, &QRadioButton::clicked, this, [this]() {
        if (ui->exit_ALL->isChecked()) {
            ui->exit_DAEGU->setEnabled(false);
            ui->exit_DAEJUN->setEnabled(false);
            ui->exit_SEOUL->setEnabled(false);
            ui->exit_BUSAN->setEnabled(false);
        } else {
            ui->exit_DAEGU->setEnabled(true);
            ui->exit_DAEJUN->setEnabled(true);
            ui->exit_SEOUL->setEnabled(true);
            ui->exit_BUSAN->setEnabled(true);
        }
    });

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

    //dbManager->fetchData("http://192.168.1.15:8080/records"); // 데이터 요청
}

highPassWindow::~highPassWindow() {
    delete dataList;
    delete ui;
}



void highPassWindow::on_search_Button_clicked()
{
    QDate startDate = ui->date_Start->date();
    QDate endDate = ui->date_End->date();
    QString searchText = ui->search_Line->text().trimmed();

    QString baseUrl = "http://127.0.0.1:8080/records";
    QUrl url(baseUrl);

    QUrlQuery queryParams;
    queryParams.addQueryItem("startDate", startDate.toString("yyyy-MM-dd"));
    queryParams.addQueryItem("endDate", endDate.toString("yyyy-MM-dd"));

    // 검색 텍스트가 비어 있지 않을 때만 추가
    if (!searchText.isEmpty()) {
        queryParams.addQueryItem("plateNumber", searchText);
    }

    // 출발지 필터링 조건 추가
    QStringList entryLocations;
    if (!ui->entry_ALL->isChecked()) { // 전체 선택이 아닐 때만 조건 추가
        if (ui->entry_SEOUL->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_SEOUL_ENTRY);
        if (ui->entry_DAEGU->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_DAEGU_ENTRY);
        if (ui->entry_DAEJUN->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_DAEJUN_ENTRY);
        if (ui->entry_BUSAN->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_BUSAN_ENTRY);
    }
    if (!entryLocations.isEmpty()) {
        queryParams.addQueryItem("entryGate", entryLocations.join(","));
    }

    // 도착지 필터링 조건 추가
    QStringList exitLocations;
    if (!ui->exit_ALL->isChecked()) { // 전체 선택이 아닐 때만 조건 추가
        if (ui->exit_SEOUL->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_SEOUL_EXIT);
        if (ui->exit_DAEGU->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_DAEGU_EXIT);
        if (ui->exit_DAEJUN->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_DAEJUN_EXIT);
        if (ui->exit_BUSAN->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_BUSAN_EXIT);
    }
    if (!exitLocations.isEmpty()) {
        queryParams.addQueryItem("exitGate", exitLocations.join(","));
    }

    url.setQuery(queryParams);
    qDebug() << queryParams.toString();

    dbManager->fetchData(url.toString());
}

