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
        /*GATE_SEOUL_ENTRY = 1,
        GATE_SEOUL_EXIT = 2,
        GATE_DAEJUN_ENTRY = 3,
        GATE_DAEJUN_EXIT = 4,
        GATE_DAEGU_ENTRY = 5,
        GATE_DAEGU_EXIT = 6,
        GATE_BUSAN_ENTRY = 7,
        GATE_BUSAN_EXIT = 8
        */
        GATE_SEOUL = 1,
        GATE_DAEJUN = 2,
        GATE_DAEGU = 3,
        GATE_BUSAN = 4,
        GATE_GWANGJU = 5
    };
    int getGateFee(int gateNumber) const; // 특정 게이트 요금을 반환
    void fetchGateFees();
    QString getServerUrl() const;

signals:
    void dataReady(const QList<QList<QVariant>> &data);
    void updatePageNavigation(int totalRecords);
    void emailRegistrationFinished(const QJsonObject &response); // 성공 시그널
    void emailRegistrationError(const QString &error);          // 에러 시그널

public slots:
    bool setServerUrl(const QString &ipAddress);
    void handleEmailData(const QJsonObject &json); // JSON 데이터를 처리하는 슬롯

private slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QList<QVariant> extractRowData(const QJsonObject &obj);
    QMap<int, int> gateFeeMap; // http://127.0.0.1:8080/gatefees
    QMap<QNetworkReply*, QString> requestMap; // 요청과 URL 매핑

    void parseGateFees(const QByteArray &data); // Gate 요금을 파싱하여 저장

    QMap<int, QString> gateMap = {
        {GATE_SEOUL, "서울"},
        {GATE_DAEJUN, "대전"},
        {GATE_DAEGU, "대구"},
        {GATE_BUSAN, "부산"},
        {GATE_GWANGJU, "광주"},
    };
    QString serverUrl;


};

#endif // DATABASEMANAGER_H
