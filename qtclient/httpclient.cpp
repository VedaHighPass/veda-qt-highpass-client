#include "httpclient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>

HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    //connect(manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
}
HttpClient::HttpClient()
{
}

//void createCamera(const QString& cameraName, const QString& rtspUrl) {

//}


//void HttpClient::addCamera(const QString &cameraName, const QString &rtspUrl)
//{

//    // 서버 URL 설정
//    QUrl serverUrl("http://192.168.0.26:8080/camera"); // 서버의 REST API URL
//    QNetworkRequest request(serverUrl);
//    // 헤더 설정
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//    // JSON 데이터 생성
//    QJsonObject jsonObj;
//    jsonObj["Camera_Name"] = cameraName;
//    jsonObj["Camera_RTSP_URL"] = rtspUrl;
//    QJsonDocument jsonDoc(jsonObj);
//    // POST 요청 전송
//    manager->post(request, jsonDoc.toJson());
//    qDebug() << "POST Request Sent: " << jsonDoc.toJson();
//}


void HttpClient::addCamera(const QString &cameraName, const QString &rtspUrl)
{
    QUrl serverUrl("http://192.168.0.26:8080/camera"); // 서버 URL
      QNetworkRequest request(serverUrl);
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); // 헤더 설정

      // JSON 객체 생성
      QJsonObject jsonObj;
      jsonObj["Camera_Name"] = cameraName;
      jsonObj["Camera_RTSP_URL"] = rtspUrl;
      QJsonDocument jsonDoc(jsonObj);
      qDebug() << "Generated JSON for POST:" << jsonDoc.toJson(QJsonDocument::Compact);

      // POST 요청 전송
      QNetworkReply *reply = manager->post(request, jsonDoc.toJson());

      // 응답이 오면 onReplyFinished 슬롯으로 전달
      connect(reply, &QNetworkReply::finished, this, &HttpClient::onCameraAdded);
       emit signal_finishAdd();
      // 요청 전송 디버그 로그
      qDebug() << "POST Request Sent:" << jsonDoc.toJson();
}

void HttpClient::loadCameras()
{
    QUrl url("http://192.168.0.26:8080/cameras");
     QNetworkRequest request(url);
     QNetworkReply *reply = manager->get(request);

     // 응답이 오면 onCamerasLoaded 슬롯으로 전달
     connect(reply, &QNetworkReply::finished, this, &HttpClient::onCamerasLoaded);

     // 요청 전송 디버그 로그
     qDebug() << "GET Request Sent to: " << url.toString();
}



void HttpClient::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        // 성공 시 응답 처리
        QByteArray responseData = reply->readAll();
        qDebug() << "Response Received:" << responseData;
    } else {
        // 오류 처리
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater(); // 리소스 해제
}
//void HttpClient::loadCameras()
//{
//    QUrl url("http://192.168.0.26:8080/cameras");
//    QNetworkRequest request(url);
//    QNetworkReply *reply = manager->get(request);
//   // connect(reply,  SIGNAL(finished(QNetworkReply *reply)), this, SLOT(onCamerasLoaded(QNetworkReply *reply)));
//    connect(reply, &QNetworkReply::finished, this, [this, reply]() {onCamerasLoaded(reply);});
//}
void HttpClient::onCameraAdded()
{
    // POST 요청에 대한 응답 처리
     QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
     if (reply->error() == QNetworkReply::NoError) {
         QByteArray responseData = reply->readAll();
         qDebug() << "Camera Added Response Received:" << responseData;
     } else {
             qDebug() << "Network error:" << reply->errorString();
         qDebug() << "Error adding camera:" << reply->errorString();
     }
     reply->deleteLater();
}
void HttpClient::onCamerasLoaded()
{
    // GET 요청에 대한 응답 처리
       QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

       // 응답 상태 체크
       if (reply->error() == QNetworkReply::NoError) {
           QByteArray responseData = reply->readAll();
           qDebug() << "Cameras Loaded Response Received:" << responseData;

           if (responseData.isEmpty()) {
               qDebug() << "Received empty response data from server.";
           } else {
               QJsonDocument doc = QJsonDocument::fromJson(responseData);
               if (doc.isArray()) {
                   QJsonArray cameraArray = doc.array();
                   for (const QJsonValue &value : cameraArray) {
                       QJsonObject cameraObject = value.toObject();
                       qDebug() << "Camera ID:" << cameraObject["Camera_ID"].toInt()
                                << "Camera Name:" << cameraObject["Camera_Name"].toString()
                                << "RTSP URL:" << cameraObject["Camera_RTSP_URL"].toString();
                       map_url.insert(cameraObject["Camera_Name"].toString(), cameraObject["Camera_RTSP_URL"].toString());
                   }
               } else {
                   qDebug() << "Response is not an array.";
               }
           }
       } else {
           qDebug() << "Failed to load camera list: " << reply->errorString();
       }
       emit signal_finishload();
       reply->deleteLater(); // 메모리 해제
   //  reply->deleteLater(); // 메모리 해제
//    if (reply->error() == QNetworkReply::NoError) {
//           qDebug() << "Load started.";
//           QByteArray responseData = reply->readAll();
//           qDebug() << "Response Data:" << responseData;

//           QJsonParseError parseError;
//           QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
//           if (parseError.error != QJsonParseError::NoError) {
//               qWarning() << "JSON Parse Error:" << parseError.errorString();
//               return;
//           }

//           if (doc.isArray()) {
//               QJsonArray cameraArray = doc.array();
//               qDebug() << "Camera Array Size:" << cameraArray.size();
//               for (int i = 0; i < cameraArray.size(); ++i) {
//                   qDebug() << QString("cameraArray[%1]").arg(i);
//               }
//           } else if (doc.isObject()) {
//               QJsonObject obj = doc.object();
//               if (obj.contains("cameraArray") && obj["cameraArray"].isArray()) {
//                   QJsonArray cameraArray = obj["cameraArray"].toArray();
//                   qDebug() << "Camera Array Size:" << cameraArray.size();
//                   for (int i = 0; i < cameraArray.size(); ++i) {
//                       qDebug() << QString("cameraArray[%1]").arg(i);
//                   }
//               } else {
//                   qDebug() << "'cameraArray' not found or it's not an array.";
//               }
//           } else {
//               qDebug() << "Unexpected JSON structure.";
//           }
//       } else {
//           qDebug() << "Camera list loading failed: " << reply->errorString();
//       }
//       reply->deleteLater();
}
QMap<QString,QString> HttpClient::getCameraUrlMap()
{
    qDebug()<<"getCameraMap!!!";

    return map_url;
}
