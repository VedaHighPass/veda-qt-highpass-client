#ifndef STREAM_UI_H
#define STREAM_UI_H

#include <QWidget>
#include "rtpclient.h"
class  HttpClient;
namespace Ui {
class stream_ui;
}

class stream_ui : public QWidget
{
    Q_OBJECT

public:
   explicit stream_ui(QWidget *parent,HttpClient* httpCli, QString stream_url);
   explicit stream_ui(QWidget *parent=nullptr, HttpClient* httpCli=nullptr);
    ~stream_ui() override;
    rtpClient* rtpCli;
private:
    Ui::stream_ui *ui;
    QString url;
    HttpClient*httpClient;
protected:
    void resizeEvent(QResizeEvent *event) override;
public slots:
    void on_startBtn_clicked();
private slots:
    void slot_push_pause();
    void slot_push_restart();
    void slot_push_disconnect();
    void slot_streaming_start();
    void slot_video_start();
    void slot_streaming_fail();

signals:
    void signal_clikQuit();
    void signal_stream_ui_del(stream_ui*);
};

#endif // STREAM_UI_H
