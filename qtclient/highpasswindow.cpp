#include "highpasswindow.h"
#include "ui_highpasswindow.h"
#include "datalist.h" // dataList 포함
#include "databasemanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QButtonGroup>
#include <QLayoutItem>
#include "sendemail.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

QGraphicsDropShadowEffect* createShadowEffect(int r, int x, int y, int alpha);

highPassWindow::highPassWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::highPassWindow)
    , dataList(nullptr) // 초기화
    , dbManager(nullptr)
{
    ui->setupUi(this);
    updateIcon();
    ui->search_Line->setPlaceholderText("Search...");
    ui->tableView->setFocusPolicy(Qt::NoFocus);

    // LineEdit에 IP 형식 입력 제한 설정
    QRegularExpression ipRegex(R"((\d{1,3}\.){3}\d{1,3})"); // IPv4 형식
    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ui->db_address_Line->setValidator(ipValidator);

    // shadow effect
    QList<QWidget*> widgets = {
        ui->top_background,
        ui->mid_background,
        ui->Bill_Button,
        ui->mail_Button,
        ui->register_Button,
        ui->tableView
    };

    for (QWidget* widget : widgets) {
        widget->setGraphicsEffect(createShadowEffect(50,1,1,50));
    }

    // entrygate button set
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

    // exitgate button set
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

    // DB 버튼 클릭 시 연결
    connect(ui->DB_Button, &QPushButton::clicked, this, &highPassWindow::initializeDatabaseManager);
    /*
    dbManager = new DatabaseManager(this);
    connect(dbManager, &DatabaseManager::dataReady, this, [this](const QList<QList<QVariant>> &data) {
       dataList->populateData(data);
    });
    connect(dbManager, &DatabaseManager::updatePageNavigation, this, &highPassWindow::updatePageButtons);
    ui->db_address_Line->setText("127.0.0.1");
    connect(ui->db_address_Line, &QLineEdit::textChanged, dbManager, &DatabaseManager::setServerUrl);
    */
}

highPassWindow::~highPassWindow() {
    delete dataList;
    delete ui;
}



void highPassWindow::on_search_Button_clicked()
{
    if (!dbManager){
        qDebug() <<  QMessageBox::warning(this, "Server Connecting Error", "Cannot connect to server. Please set the server address first.");
        return;
    }

    QDate startDate = ui->date_Start->date();
    QDate endDate = ui->date_End->date();
    QString searchText = ui->search_Line->text().trimmed();

    QString baseUrl = dbManager->getServerUrl() + "records";
    QUrl url(baseUrl);

    currentQueryParams = QUrlQuery(); // 기존 쿼리 초기화
    currentQueryParams.addQueryItem("startDate", startDate.toString("yyyy-MM-dd"));
    currentQueryParams.addQueryItem("endDate", endDate.toString("yyyy-MM-dd"));

    // 검색 텍스트 추가
    if (!searchText.isEmpty()) {
        currentQueryParams.addQueryItem("plateNumber", searchText);
    }

    // 출발지 조건 추가
    QStringList entryLocations;
    if (!ui->entry_ALL->isChecked()) {
        if (ui->entry_SEOUL->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_SEOUL);
        if (ui->entry_DAEGU->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_DAEGU);
        if (ui->entry_DAEJUN->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_DAEJUN);
        if (ui->entry_BUSAN->isChecked()) entryLocations << QString::number(DatabaseManager::GATE_BUSAN);
    }
    if (!entryLocations.isEmpty()) {
        currentQueryParams.addQueryItem("entryGate", entryLocations.join(","));
    }

    // 도착지 조건 추가
    QStringList exitLocations;
    if (!ui->exit_ALL->isChecked()) {
        if (ui->exit_SEOUL->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_SEOUL);
        if (ui->exit_DAEGU->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_DAEGU);
        if (ui->exit_DAEJUN->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_DAEJUN);
        if (ui->exit_BUSAN->isChecked()) exitLocations << QString::number(DatabaseManager::GATE_BUSAN);
    }
    if (!exitLocations.isEmpty()) {
        currentQueryParams.addQueryItem("exitGate", exitLocations.join(","));
    }

    // 페이지 관련 정보 초기화
    currentPage = 1;
    currentQueryParams.addQueryItem("pageSize", QString::number(pageSize));
    currentQueryParams.addQueryItem("page", QString::number(currentPage));

    // URL 갱신
    url.setQuery(currentQueryParams);
    qDebug() << "Query URL:" << url.toString();

    // 데이터 요청
    dbManager->fetchData(url.toString());
}

void highPassWindow::updatePageButtons(int totalRecords) {
    int totalPages = (totalRecords + pageSize - 1) / pageSize; // 총 페이지 수 계산

    // 기존 버튼 제거
    QLayoutItem *child;
    while ((child = ui->pageLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QString buttonStyle = R"(
        QPushButton {
            border-radius: 5px;
            background-color: rgb(255, 255, 255);
            width: 50px;
            height: 40px;
            color: rgb(35, 76, 159);
            border: 1px solid #ccc;
        }
        QPushButton:pressed {
            border: 1px solid #333333;
            background-color: #222222;
            color: white;
        }
        QPushButton:hover {
            background-color: rgb(35, 76, 159);
            color: white;
        }
        QPushButton:checked {
            background-color: rgb(35, 76, 159);
            color: white;
            border: 1px solid rgb(35, 76, 159);
        }
    )";

    // 새로운 버튼 추가
    for (int i = 1; i <= totalPages; ++i) {
        QPushButton *pageButton = new QPushButton(QString::number(i), this);
        pageButton->setCheckable(true);

        // 버튼 크기 제한 설정
        pageButton->setMaximumWidth(50);
        pageButton->setStyleSheet(buttonStyle);

        // 현재 페이지 버튼 스타일 설정
        if (i == currentPage) {
            pageButton->setChecked(true); // 현재 페이지는 선택된 상태
        }

        ui->pageLayout->addWidget(pageButton);

        // 버튼 클릭 시 데이터 요청
        connect(pageButton, &QPushButton::clicked, this, [this, i]() {
            if (i != currentPage) { // 현재 페이지와 다른 버튼 클릭 시에만 요청
                currentPage = i; // 현재 페이지 업데이트
                currentQueryParams.removeQueryItem("page");
                currentQueryParams.addQueryItem("page", QString::number(currentPage));

                QString baseUrl = dbManager->getServerUrl() + "records";
                QUrl url(baseUrl);
                url.setQuery(currentQueryParams);

                dbManager->fetchData(url.toString()); // 새 데이터 요청
            }
        });
    }
}

void highPassWindow::on_mail_Button_clicked()
{
    dataList->getCheckedClients();
    SendEmail *mailWidget = new SendEmail(this); // 항상 새 객체 생성
    mailWidget->setWindowFlags(Qt::Window); // 독립 창으로 설정

    mailWidget->setAttribute(Qt::WA_DeleteOnClose); // 창 닫힐 때 자동 삭제
    mailWidget->show();
    mailWidget->raise();
    mailWidget->activateWindow();

}

QGraphicsDropShadowEffect* createShadowEffect(int r, int x, int y, int alpha) {
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(r);  // 그림자 블러 정도
    shadowEffect->setOffset(x, y);    // 그림자 위치
    shadowEffect->setColor(QColor(0, 0, 0, alpha)); // 투명도 포함 색상
    return shadowEffect;
}

void highPassWindow::initializeDatabaseManager()
{
    QString ipAddress = ui->db_address_Line->text().trimmed();

    if (!validateIpAddress(ipAddress)) {
        QMessageBox::warning(this, "Invalid IP", "Please enter a valid IP address in the format xxx.xxx.xxx.xxx.");
        return;
    }

    if (dbManager) {
        delete dbManager;
        dbManager = nullptr;
    }
    dbManager = new DatabaseManager(this);
    dbManager->setServerUrl(ipAddress);

    // fetchGateFees 호출
    dbManager->fetchGateFees();

    // 시그널 연결
    connect(dbManager, &DatabaseManager::dataReady, this, [this](const QList<QList<QVariant>> &data) {
        dataList->populateData(data);
    });
    connect(dbManager, &DatabaseManager::updatePageNavigation, this, &highPassWindow::updatePageButtons);

    QMessageBox::information(this, "Database Connected", "Database connection initialized successfully.");
    updateIcon();

    /*dbManager->fetchGateFees([this](bool success) {
        if (success) {
            connect(dbManager, &DatabaseManager::dataReady, this, [this](const QList<QList<QVariant>> &data) {
                dataList->populateData(data);
            });
            connect(dbManager, &DatabaseManager::updatePageNavigation, this, &highPassWindow::updatePageButtons);

            QMessageBox::information(this, "Database Connected", "Database connection initialized successfully.");
        } else {
            QMessageBox::warning(this, "Server Connecting Error", "Cannot connect to server. Please check the IP address and try again.");
        }
    });*/
}

bool highPassWindow::validateIpAddress(const QString &ipAddress) const
{
    QRegularExpression ipRegex(R"(^(\d{1,3}\.){3}\d{1,3}$)");
    QRegularExpressionMatch match = ipRegex.match(ipAddress);

    if (!match.hasMatch()) {
        return false; // 형식이 맞지 않으면 유효하지 않음
    }

    QStringList octets = ipAddress.split('.');
    for (const QString &octet : octets) {
        bool ok;
        int value = octet.toInt(&ok);
        if (!ok || value < 0 || value > 255) {
            return false; // 각 옥텟이 0~255 범위를 벗어나면 유효하지 않음
        }
    }
    return true; // 모든 조건 만족 시 유효
}

void highPassWindow::updateIcon() {
    if (dbManager) {
        ui->icon_Label->setPixmap(QPixmap(":/images/images/green_icon.png"));
    } else {
        ui->icon_Label->setPixmap(QPixmap(":/images/images/red_icon.png"));
    }
}
