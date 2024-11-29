#include "databasemanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "datalist.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), networkManager(new QNetworkAccessManager(this)){
    connect(networkManager, &QNetworkAccessManager::finished, this, &DatabaseManager::handleNetworkReply);
}

DatabaseManager::~DatabaseManager() {}

void DatabaseManager::fetchData(const QString &url) {
    QNetworkRequest request;
    request.setUrl(QUrl(url)); // URL 설정
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkManager->get(request); // GET 요청
}

void DatabaseManager::handleNetworkReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    // JSON 문서를 파싱
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isArray()) {
        qDebug() << "Invalid JSON format. Expected an array.";
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    QList<QList<QVariant>> data;


    // JSON 배열의 각 객체 처리
    for (const QJsonValue &value : jsonArray) {
        if (!value.isObject()) {
            qDebug() << "Skipping non-object JSON value.";
            continue;
        }

        QJsonObject obj = value.toObject();
        data.append(extractRowData(obj));

    }
    qDebug() << data;

    emit dataReady(data); // 데이터 준비 완료 신호
}

QList<QVariant> DatabaseManager::extractRowData(const QJsonObject &obj) {

    QList<QVariant> row(DataList::COL_COUNT, QVariant()); // 열 개수만큼 초기화

    // 열 상수에 따라 데이터 추가
    row[DataList::COL_CHECKBOX] = QVariant(); // CheckBox는 비워둠

    // JSON 키 매핑 및 기본값 처리
    row[DataList::COL_PHOTO] = obj.contains("Photo") ? obj["Photo"].toString() : "No Photo";
    row[DataList::COL_REGISTRATION] = obj.contains("Registration") ? obj["Registration"].toInt() : 0;
    row[DataList::COL_BILL] = obj.contains("Bill") ? obj["Bill"].toInt() : 0;
    row[DataList::COL_PAYMENT] = obj.contains("Payment") ? obj["Payment"].toInt() : 0;
    row[DataList::COL_PLATENUM] = obj.contains("PlateNumber") ? obj["PlateNumber"].toString() : "Unknown";

    // GateNumber -> GateName 변환
    int entryGateNum = obj.contains("EntryGateNumber") ? obj["EntryGateNumber"].toInt() : -1;
    int exitGateNum = obj.contains("ExitGateNumber") ? obj["ExitGateNumber"].toInt() : -1;

    row[DataList::COL_START_LOCATION] = entryGateNum >= 0 ? gateMap.value(entryGateNum, "Unknown Entry Gate") : "-";
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

    // 기타 데이터
    row[DataList::COL_BILL_DATE] = obj.contains("BillDate") ? obj["BillDate"].toString("-") : "-";
    row[DataList::COL_UNPAIDFEE] = obj.contains("UnpaidFee") ? obj["UnpaidFee"].toInt() : 0;

    return row;
}
