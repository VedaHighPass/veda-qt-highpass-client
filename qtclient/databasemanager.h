#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QVariant>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    void fetchData(const QString &url);

    enum GateType {
        GATE_SEOUL_ENTRY = 1,
        GATE_SEOUL_EXIT = 2,
        GATE_DAEJUN_ENTRY = 3,
        GATE_DAEJUN_EXIT = 4,
        GATE_DAEGU_ENTRY = 5,
        GATE_DAEGU_EXIT = 6,
        GATE_BUSAN_ENTRY = 7,
        GATE_BUSAN_EXIT = 8
    };


signals:
    void dataReady(const QList<QList<QVariant>> &data);
    void updatePageNavigation(int totalRecords);

private slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QList<QVariant> extractRowData(const QJsonObject &obj);


    QMap<int, QString> gateMap = {
        {GATE_SEOUL_ENTRY, "서울"},
        {GATE_SEOUL_EXIT, "서울"},
        {GATE_DAEJUN_ENTRY, "대전"},
        {GATE_DAEJUN_EXIT, "대전"},
        {GATE_DAEGU_ENTRY, "대구"},
        {GATE_DAEGU_EXIT, "대구"},
        {GATE_BUSAN_ENTRY, "부산"},
        {GATE_BUSAN_EXIT, "부산"},
    };
};

#endif // DATABASEMANAGER_H
