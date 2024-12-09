#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H

#include <QWidget>
#include <QMap>
class HttpClient;
class QTextEdit;
class rtpClient;
class stream_ui;

namespace Ui {
class videoStream;
}

class videoStream : public QWidget
{
    Q_OBJECT

public:
    explicit videoStream(QWidget *parent = nullptr);
    ~videoStream();

private slots:

    void slot_ffmpeg_debug(QString error,rtpClient* textedit_key);
    void showContextMenu(const QPoint& pos);
    void addNewTab();
    void slot_tab_del(stream_ui*);
    void addRegistedCam();
    //void deletSubWind(stream_ui* delIndex);
private:
    Ui::videoStream *ui;
    HttpClient* httpCli;
    QMap <rtpClient*,QTextEdit*> map_textedit;
    QMap <stream_ui*, QWidget*> map_stream_ui;
    QMap <QString, QString> map_url;

signals:
    void start_stream();
    //void signal_clikQuit();
   // void send_url(QString url);
};

#endif // VIDEOSTREAM_H
