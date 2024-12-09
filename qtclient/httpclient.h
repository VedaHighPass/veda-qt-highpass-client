#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QMap>

class QNetworkAccessManager;
class QNetworkReply;
class HttpClient: public QObject
{
    Q_OBJECT
public:
    HttpClient(QObject *parent);
    void addCamera(const QString &cameraName, const QString &rtspUrl);
    void loadCameras();
    QMap<QString,QString> getCameraUrlMap();
    HttpClient();

private:
      QNetworkAccessManager* manager;
      QMap<QString,QString> map_url;

private slots:
       void onCameraAdded();
       void onCamerasLoaded();
       void onReplyFinished(QNetworkReply* reply);
signals:
       void signal_finishload();
       void signal_finishAdd();
};

#endif // HTTPCLIENT_H
