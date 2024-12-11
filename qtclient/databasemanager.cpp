#include "databasemanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "datalist.h"
#include <QLocale>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QVariant>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), networkManager(new QNetworkAccessManager(this)) {
    //connect(networkManager, &QNetworkAccessManager::finished, this, &DatabaseManager::handleNetworkReply);
}

DatabaseManager::~DatabaseManager() {}

bool DatabaseManager::setServerUrl(const QString &ipAddress) {
    if (ipAddress.isEmpty()) {
        qDebug() << "Empty IP address.";
        return false;
    }

    serverUrl = QString("https://%1:8080/").arg(ipAddress.trimmed());
    qDebug() << "Updated server URL:" << serverUrl;

    return true;
}

void DatabaseManager::fetchData(const QString &url) {
    QNetworkRequest request;
    request.setUrl(QUrl(url)); // URL 설정
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //networkManager->get(request); // GET 요청
    QNetworkReply* reply = networkManager->get(request); // GET 요청
    requestMap[reply] = url; // 요청과 URL 매핑
    qDebug() << "fetch Data";
}

void DatabaseManager::fetchGateFees() {
    QString url = serverUrl + "gatefees";
    QUrl qUrl(url);

    if (!qUrl.isValid()) {  // URL 유효성 검사
        qDebug() << "Invalid URL:" << qUrl;
        return;
    }
    QNetworkRequest request(qUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkManager->get(request);
    connect(networkManager, &QNetworkAccessManager::finished, this, &DatabaseManager::handleNetworkReply);
}

int DatabaseManager::getGateFee(int gateNumber) const {
    return gateFeeMap.value(gateNumber, 0); // 기본값 0 반환
}

void DatabaseManager::handleNetworkReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    //QString url = requestMap.take(reply); // 요청에 대응되는 URL 가져오기
    QUrl requestUrl = reply->request().url();  // 요청 URL 확인
    reply->deleteLater();

    reply->deleteLater();

    // URL에 따라 처리 분기
    if (requestUrl.toString().contains("gatefees")) {
        parseGateFees(responseData);  // Gate 요금 데이터 파싱
    } else { // JSON 문서를 파싱

        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isObject()) {
            qDebug() << "Invalid JSON format. Expected an object.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        // 총 레코드 수 가져오기
        int totalRecords = jsonObj.value("totalRecords").toInt();

        // 데이터 배열 파싱
        QJsonArray jsonArray = jsonObj.value("data").toArray();
        QList<QList<QVariant>> data;
        for (const QJsonValue &value : jsonArray) {
            if (!value.isObject()) {
                qDebug() << "Skipping non-object JSON value.";
                continue;
            }

            QJsonObject obj = value.toObject();
            data.append(extractRowData(obj));
        }

        // 페이지 버튼 갱신
        emit dataReady(data);
        emit updatePageNavigation(totalRecords); // 총 레코드 수와 현재 페이지 전달
    }
}

QList<QVariant> DatabaseManager::extractRowData(const QJsonObject &obj) {
    QList<QVariant> row(DataList::COL_COUNT, QVariant()); // 열 개수만큼 초기화

    /// Path 처리
    int totalFee = 0; // 총 요금 초기화
    if (obj.contains("Path")) {
        QString pathString = obj["Path"].toString(); // Path 값을 가져옴
        QStringList gateNumbers = pathString.split(",", Qt::SkipEmptyParts); // Path를 콤마로 분리

        // 각 게이트 번호의 요금을 합산
        for (const QString &gateNumberStr : gateNumbers) {
            bool ok;
            int gateNumber = gateNumberStr.toInt(&ok); // 숫자로 변환
            if (ok && gateFeeMap.contains(gateNumber)) {
                totalFee += gateFeeMap[gateNumber]; // 요금 추가
            }
        }

        // 경로를 -> 형식으로 변환
        QStringList gateNames;
        for (const QString &gateNumberStr : gateNumbers) {
            bool ok;
            int gateNumber = gateNumberStr.toInt(&ok);
            if (ok && gateMap.contains(gateNumber)) {
                gateNames.append(gateMap[gateNumber]); // Gate 이름으로 매핑
            }
        }
        row[DataList::COL_PATH] = gateNames.join("->");
    } else {
        row[DataList::COL_PATH] = "-";
    }

    // UnpaidFee에 총 요금 추가
    row[DataList::COL_UNPAIDFEE] = QLocale(QLocale::English).toString(totalFee)  + "\\";

    // PlateNumber 가져오기
    QString plateNumber = obj.contains("PlateNumber") ? obj["PlateNumber"].toString() : "Unknown";
    // 이미지 경로 생성
    QString entryImagePath = QString("%1images/%2/entry.jpg").arg(serverUrl).arg(plateNumber);

    // 열 상수에 따라 데이터 추가
    row[DataList::COL_CHECKBOX] = QVariant(); // CheckBox는 비워둠

    // JSON 키 매핑 및 기본값 처리
    row[DataList::COL_PHOTO] = entryImagePath;

    row[DataList::COL_EMAIL] = obj.contains("Email") && !obj["Email"].toString().isEmpty()
                                   ? obj["Email"].toString()
                                   : "-";
    row[DataList::COL_PLATENUM] = obj.contains("PlateNumber") ? obj["PlateNumber"].toString() : "Unknown";

    // GateNumber -> GateName 변환
    int entryGateNum = obj.contains("EntryGateNumber") ? obj["EntryGateNumber"].toInt() : -1;
    int exitGateNum = obj.contains("ExitGateNumber") ? obj["ExitGateNumber"].toInt() : -1;

    row[DataList::COL_START_LOCATION] = entryGateNum >= 0 ? gateMap.value(entryGateNum, "-") : "-";
    row[DataList::COL_END_LOCATION] = exitGateNum >= 0 ? gateMap.value(exitGateNum, "-") : "-";

    // EntryTime 처리
    QString entryTimeStr = obj.value("EntryTime").toString();
    if (entryTimeStr.contains('.')) {
        entryTimeStr = entryTimeStr.section('.', 0, 0); // . 이전까지만 가져오기
    }
    QDateTime entryTime = QDateTime::fromString(entryTimeStr, "yyyy-MM-dd HH:mm:ss");
    row[DataList::COL_START_DATE] = entryTime.isValid()
                                        ? entryTime.toString("yyyy-MM-dd HH:mm")
                                        : "-";

    // ExitTime 처리 (NULL 시 "-")
    QString exitTimeStr = obj.value("ExitTime").toString("-");
    if (exitTimeStr.contains('.')) {
        exitTimeStr = exitTimeStr.section('.', 0, 0); // . 이전까지만 가져오기
    }
    QDateTime exitTime = QDateTime::fromString(exitTimeStr, "yyyy-MM-dd HH:mm:ss");
    row[DataList::COL_END_DATE] = exitTime.isValid()
                                      ? exitTime.toString("yyyy-MM-dd HH:mm")
                                      : "-";

    return row;
}

void DatabaseManager::parseGateFees(const QByteArray &data) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON format: Expected an object.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    gateFeeMap.clear(); // 기존 데이터 초기화

    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it){
        int gateNumber = it.key().toInt(); // 키를 정수로 변환
        int gateFee = it.value().toInt(); // 값을 정수로 가져옴
        gateFeeMap[gateNumber] = gateFee;
    }
    qDebug() << "Gate fees loaded:" << gateFeeMap;
}

QString DatabaseManager::getServerUrl() const {
    return serverUrl;
}

void DatabaseManager::handleEmailData(const QJsonObject &json) {
    if (serverUrl.isEmpty()) {
        emit emailRegistrationError("Server URL is not set.");
        return;
    }

    // URL 설정
    QString url = serverUrl + "emails";
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // JSON 직렬화 및 POST 요청
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    QNetworkReply *reply = networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
            emit emailRegistrationFinished(responseDoc.object()); // 성공 시 응답 전달
        } else {
            emit emailRegistrationError(reply->errorString()); // 에러 시 에러 메시지 전달
        }
        reply->deleteLater();
    });
}
