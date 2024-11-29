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
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
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

    /*
    // 키-열 매핑 정의
    QMap<QString, int> columnMapping = {
        {"BillID", 0},
        {"EnterGateID", 1},
        {"EnterGateRecordID", 2},
        {"ExitGateID", 3},
        {"ExitGateRecordID", 4},
        {"HasPaid", 5},
        {"PayDate", 6},
        {"PlateNumber", 7},
        {"Price", 8}
    };*/

    // JSON 배열의 각 객체 처리
    for (const QJsonValue &value : jsonArray) {
        if (!value.isObject()) {
            qDebug() << "Skipping non-object JSON value.";
            continue;
        }

        QJsonObject obj = value.toObject();
        data.append(extractRowData(obj));

        /*
        // 키-열 매핑에 따라 값 추가
        for (auto it = columnMapping.begin(); it != columnMapping.end(); ++it) {
            QString key = it.key();
            int col = it.value();


            if (obj.contains(key)) {
                QJsonValue jsonValue = obj[key];

                qDebug() << jsonValue << " " << '\n';

                // 타입에 따라 값 변환
                if (jsonValue.isBool()) {
                    row[col] = jsonValue.toBool();
                } else if (jsonValue.isDouble()) {
                    row[col] = jsonValue.toDouble();
                } else if (jsonValue.isString()) {
                    row[col] = jsonValue.toString();
                } else if (jsonValue.isNull()) {
                    row[col] = QVariant(); // Null 처리
                } else {
                    row[col] = QVariant("Unsupported"); // 미지원 타입 처리
                }
            }
        }
        */
    }

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

    row[DataList::COL_START_LOCATION] = obj.contains("GateNumber") ? gateMap.value(obj["GateNumber"].toInt(), "Unknown") : "Unknown";

    //row[DataList::COL_START_DATE] = obj.contains("EntryTime") ? obj["EntryTime"].toString() : "N/A";
    if (obj.contains("EntryTime")) {
        QString entryTimeStr = obj["EntryTime"].toString();
        if (entryTimeStr.contains('.')) {
            entryTimeStr = entryTimeStr.section('.', 0, 0); // . 이전까지만 가져오기
        }

        QDateTime entryTime = QDateTime::fromString(entryTimeStr, "yyyy_MM_dd_HH:mm:ss");
        // 유효한 시간일 경우 변환, 그렇지 않으면 기본값 설정
        row[DataList::COL_START_DATE] = entryTime.isValid()
                                            ? entryTime.toString("yyyy-MM-dd HH:mm")
                                            : "Invalid Date";
    } else {
        row[DataList::COL_START_DATE] = "N/A"; // EntryTime이 없는 경우
    }

    row[DataList::COL_END_LOCATION] = obj.contains("EndLocation") ? obj["EndLocation"].toString() : "N/A";
    row[DataList::COL_END_DATE] = obj.contains("EndDate") ? obj["EndDate"].toString() : "N/A";

    row[DataList::COL_BILL_DATE] = obj.contains("BillDate") ? obj["BillDate"].toString() : "N/A";
    row[DataList::COL_UNPAIDFEE] = obj.contains("UnpaidFee") ? obj["UnpaidFee"].toInt() : 0;

    return row;
}
