#include "datalist.h"
#include "checkboxdelegate.h" // CheckBoxDelegate 클래스 포함
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QNetworkRequest>
#include <QNetworkReply>

DataList::DataList(QTableView *tableView, QObject *parent)
    : tableView(tableView), gridmodel(nullptr), networkManager(new QNetworkAccessManager(this)) {
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
/*
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
*/
void DataList::populateData(const QList<QList<QVariant>> &data) {
    gridmodel->setRowCount(0); // 기존 데이터 초기화

    for (const QList<QVariant> &row : data) {
        QList<QStandardItem *> items;

        for (int i = 0; i < row.size(); ++i) {
            if (i == DataList::COL_PHOTO) { // 이미지 열인 경우
                QString imageUrl = row[i].toString(); // 이미지 URL
                QStandardItem* imageItem = new QStandardItem();
                items.append(imageItem);

                // HTTP GET 요청으로 이미지 다운로드
                QNetworkRequest request;
                request.setUrl(QUrl(imageUrl)); // URL 설정
                QNetworkReply* reply = networkManager->get(request); // GET 요청
                reply->setProperty("row", gridmodel->rowCount());
                reply->setProperty("column", COL_PHOTO);

                connect(reply, &QNetworkReply::finished, this, &DataList::onImageDownloaded);
            } else {
                // 일반 데이터 열 처리
                items.append(new QStandardItem(row[i].toString()));
            }
        }

        gridmodel->appendRow(items);
    }

    for (int row = 0; row < gridmodel->rowCount(); ++row) {
        tableView->setRowHeight(row, 100);
    }
}

void DataList::onImageDownloaded() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender()); // 신호를 보낸 객체를 가져옴
    if (!reply) {
        qDebug() << "Invalid QNetworkReply";
        return;
    }

    QPixmap pixmap;
    if (reply->error() == QNetworkReply::NoError) {
        // 요청 성공: 다운로드한 이미지 로드
        QByteArray imageData = reply->readAll();
        pixmap.loadFromData(imageData);
    } else {
        // 요청 실패: 기본 이미지 로드
        qDebug() << "Failed to download image:" << reply->errorString();
        if (!pixmap.load(":/images/images/default_car.png")) {
            qDebug() << "Failed to load default image!";
        }
    }

    // 이미지 테이블에 설정
    int row = reply->property("row").toInt();
    int column = reply->property("column").toInt();

    if (!pixmap.isNull() && row >= 0 && column >= 0) {
        QStandardItem* item = gridmodel->item(row, column);
        if (item) {
            item->setData(QVariant(pixmap), Qt::DecorationRole);
        }
    }

    reply->deleteLater();
}

