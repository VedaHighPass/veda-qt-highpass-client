#include "datalist.h"
#include "checkboxdelegate.h" // CheckBoxDelegate 클래스 포함
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCheckBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QHBoxLayout>

DataList::DataList(QTableView *tableView, QObject *parent)
    : tableView(tableView), gridmodel(nullptr), networkManager(new QNetworkAccessManager(this)) {
}

void DataList::GridTableView() {
    QModelIndex index;
    gridmodel = new QStandardItemModel(15, 10, tableView); // 15행 10열 설정
    tableView->setModel(gridmodel);

    // 헤더 설정
    // gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_CHECKBOX, new QStandardItem(QString(" ")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_PHOTO, new QStandardItem(QString("Photo")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_PATH, new QStandardItem(QString("Path")));

    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_PLATENUM, new QStandardItem(QString("Plate Num")));

    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_START_LOCATION, new QStandardItem(QString("Entry")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_START_DATE, new QStandardItem(QString("Entry Date")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_END_LOCATION, new QStandardItem(QString("Exit")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_END_DATE, new QStandardItem(QString("Exit Date")));

    //gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_BILL_DATE, new QStandardItem(QString("Bill")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_UNPAIDFEE, new QStandardItem(QString("Due")));
    gridmodel->setHorizontalHeaderItem(DataList::Columns::COL_EMAIL, new QStandardItem(QString("Email")));


    QCheckBox* headerCheckbox = new QCheckBox(tableView);
    headerCheckbox->setText(""); // 텍스트 비우기
    headerCheckbox->setTristate(false); // 세 상태 비활성화
    headerCheckbox->setGeometry(tableView->horizontalHeader()->sectionViewportPosition(DataList::Columns::COL_CHECKBOX) + 12,
                                7, 20, 20); // 위치 및 크기 설정
    headerCheckbox->show();

    connect(headerCheckbox, &QCheckBox::stateChanged, this, [this, headerCheckbox](int state) {
        Qt::CheckState newState = static_cast<Qt::CheckState>(state);
        for (int row = 0; row < gridmodel->rowCount(); ++row) {
            QModelIndex index = gridmodel->index(row, DataList::Columns::COL_CHECKBOX);
            gridmodel->setData(index, newState, Qt::CheckStateRole);
            qDebug() << "Row" << row << "State set to:" << newState;
        }
        // 테이블 뷰 강제 갱신
        tableView->viewport()->update();
    });

    // 데이터 초기화 및 셀 크기 설정
    for (int row = 0; row < 15; row++) {
        QModelIndex index = gridmodel->index(row, DataList::Columns::COL_CHECKBOX);
        gridmodel->setData(index, Qt::Unchecked, Qt::CheckStateRole); // 체크 상태만 설정
        for (int col = 1; col < DataList::Columns::COL_COUNT; col++) {
            index = gridmodel->index(row, col, QModelIndex());
            gridmodel->setData(index, "");
        }
        tableView->setRowHeight(row, 120);
    }

    // 열 너비 조정
    tableView->setColumnWidth(DataList::Columns::COL_CHECKBOX, 40); // 체크박스 열 너비 설정
    tableView->setColumnWidth(DataList::Columns::COL_PHOTO, 170);
    tableView->setColumnWidth(DataList::Columns::COL_PATH, 150);
    tableView->setColumnWidth(DataList::Columns::COL_PLATENUM, 80);
    tableView->setColumnWidth(DataList::Columns::COL_START_LOCATION, 50);
    tableView->setColumnWidth(DataList::Columns::COL_END_LOCATION, 50);

    tableView->setColumnWidth(DataList::Columns::COL_START_DATE, 80);
    tableView->setColumnWidth(DataList::Columns::COL_END_DATE, 80);
    //tableView->setColumnWidth(DataList::Columns::COL_BILL_DATE, 80);

    tableView->setColumnWidth(DataList::Columns::COL_UNPAIDFEE,100);
    tableView->setColumnWidth(DataList::Columns::COL_EMAIL,160);

    // CheckBoxDelegate 설정
    CheckBoxDelegate *f_checkboxdelegate = new CheckBoxDelegate(tableView);
    tableView->setItemDelegateForColumn(0, f_checkboxdelegate);

    connect(gridmodel, &QStandardItemModel::itemChanged, this, [this](QStandardItem *item) {
        if (item->column() == DataList::Columns::COL_CHECKBOX) {
            updateRowColors();
        }
    });
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
        updateRowColors();
    });

}

void DataList::populateData(const QList<QList<QVariant>> &data) {
    gridmodel->setRowCount(0); // 기존 데이터 초기화

    for (const QList<QVariant> &row : data) {
        QList<QStandardItem *> items;

        for (int i = 0; i < row.size(); ++i) {
            if (i == DataList::COL_CHECKBOX) {
                // 체크박스 초기화
                QStandardItem *checkBoxItem = new QStandardItem();
                checkBoxItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled); // 체크 가능 및 활성화
                checkBoxItem->setCheckState(Qt::Unchecked); // 초기 상태를 Unchecked로 설정
                items.append(checkBoxItem);
            } else if (i == DataList::COL_PHOTO) {
                // 이미지 열 처리
                QString imageUrl = row[i].toString(); // 이미지 URL
                QStandardItem* imageItem = new QStandardItem();
                items.append(imageItem);

                // HTTP GET 요청으로 이미지 다운로드
                QNetworkRequest request;
                request.setUrl(QUrl(imageUrl)); // URL 설정
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                    // SSL 오류 무시 (개발용)
                connect(networkManager, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &errors) {
                    Q_UNUSED(errors);
                    reply->ignoreSslErrors();
                });
                QNetworkReply* reply = networkManager->get(request); // GET 요청


                // Reply에 행/열 정보를 저장 (이미지 다운로드 후 테이블에 반영하기 위해)
                reply->setProperty("row", gridmodel->rowCount());
                reply->setProperty("column", i); // 해당 열 저장
                connect(reply, &QNetworkReply::finished, this, &DataList::onImageDownloaded);
            }else {
                // 일반 데이터 열 처리
                items.append(new QStandardItem(row[i].toString()));
            }
        }

        gridmodel->appendRow(items);
    }

    for (int row = 0; row < gridmodel->rowCount(); ++row) {
        tableView->setRowHeight(row, 120);
    }
    // 데이터 갱신 후 체크박스와 배경색 업데이트
    updateRowColors();
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
        // 현재 테이블 뷰의 열 너비 가져오기
        int columnWidth = tableView->columnWidth(column);
        int rowHeight = tableView->rowHeight(row);

        // 이미지 크기를 비율 유지하며 너비에 맞게 스케일링
        pixmap = pixmap.scaled(columnWidth, rowHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QStandardItem* item = gridmodel->item(row, column);
        if (item) {
            item->setData(QVariant(pixmap), Qt::DecorationRole);
        }
    }

    reply->deleteLater();
}
QList<Client> DataList::getCheckedClients() const {
    QList<Client> checkedClients;

    // 이메일 유효성 검사 정규식
    QRegularExpression emailRegex(R"((^[\w\.-]+@[\w\.-]+\.\w{2,}$))");

    for (int row = 0; row < gridmodel->rowCount(); ++row) {
        QStandardItem *checkBoxItem = gridmodel->item(row, DataList::COL_CHECKBOX);
        if (checkBoxItem && checkBoxItem->data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            QString plateNumber = gridmodel->item(row, DataList::COL_PLATENUM)->text();
            QString email = gridmodel->item(row, DataList::COL_EMAIL)->text();
            QString dueAmount = gridmodel->item(row, DataList::COL_UNPAIDFEE)->text(); // 청구 금액 추가

            // 이미지를 가져옴
            QPixmap image;
            QStandardItem *photoItem = gridmodel->item(row, DataList::COL_PHOTO);
            if (photoItem) {
                QVariant decoration = photoItem->data(Qt::DecorationRole);
                if (decoration.canConvert<QPixmap>()) {
                    image = decoration.value<QPixmap>();
                }
            }

            // 이메일 형식 유효성 검사
            QRegularExpressionMatch match = emailRegex.match(email);
            if (match.hasMatch()) {
                Client client;
                client.plateNumber = plateNumber;
                client.email = email;
                client.dueAmount = dueAmount;
                client.image = image;
                checkedClients.append(client); // Client 구조체 추가
            } else {
                qDebug() << "Invalid email format:" << email;
            }
        }
    }
    return checkedClients;
}

void DataList::updateRowColors() {
    QItemSelectionModel* selectionModel = tableView->selectionModel();

    for (int row = 0; row < gridmodel->rowCount(); ++row) {
        QStandardItem *checkBoxItem = gridmodel->item(row, DataList::Columns::COL_CHECKBOX);
        bool isSelected = selectionModel->isRowSelected(row, QModelIndex());

        QColor bgColor;

        if (checkBoxItem && checkBoxItem->checkState() == Qt::Checked) {
            bgColor = QColor(QColor("lightblue")); // 체크박스가 선택된 행 색상
        } else if (isSelected) {
            bgColor = QColor("#ffd966"); // 선택된 경우 다른 색상
        } else {
            bgColor = (row % 2 == 0 ? QColor("#f5f5f5") : QColor("white")); // 교차 배경 유지
        }

        // 열의 배경색 설정
        for (int col = 0; col < DataList::Columns::COL_COUNT; ++col) {
            QModelIndex index = gridmodel->index(row, col);
            gridmodel->setData(index, bgColor, Qt::BackgroundRole);
        }
    }

    // 테이블 뷰 강제 갱신
    tableView->viewport()->update();
}
