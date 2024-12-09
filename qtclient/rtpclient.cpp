#include "rtpclient.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QDir>
rtpClient* rtpClient::instance() {
    static rtpClient instance;
    return &instance;
}

rtpClient::rtpClient() {
    buffer = new QByteArray;
}
rtpClient::~rtpClient()
{

   qDebug() <<"!!!!~rtpCLient()!!!!!";
   if (ffmpegProcess) {
       if (ffmpegProcess->state() != QProcess::NotRunning) {
           qDebug() << "Waiting for FFmpeg to finish...";
           finishFfmpeg(); // ?? ? μ’λ£ ?ΈμΆ?
       }
       delete ffmpegProcess;
   }
    qDebug()<<"!!!rtpCLient finished!!!!";
}

void rtpClient::readFFmpegOutput() {
    if (ffmpegProcess->state() == QProcess::Running) {
        qDebug() << "FFmpeg process is not running.";
    }

    QByteArray data = ffmpegProcess->readAllStandardOutput();
    // qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    if (data.isEmpty()) {
      //  qDebug() << "No data from ffmpeg. Check the process and arguments.";
    }
    // ?°?΄?°? ?¬κΈ? ??Έ
   // qDebug() << "Received data size:" << data.size();
    buffer->append(data);
  //  qDebug() << "Received buffer size:" << buffer.size();
    // raw RGB ?°?΄?°λ‘? κ°?? ?κ³? QImageλ‘? λ³??
    if (buffer->size() >= 640 * 480 * 3) {  // ?΄?? 640x480? ????΄ RGB? 3λ°μ΄?Έ ?½???
         QByteArray frameData = buffer->left(640 * 480 * 3);
          buffer->remove(0, 640 * 480 * 3);
        // qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        QImage image(reinterpret_cast<const uchar*>(frameData.data()), 640, 480, QImage::Format_RGB888);

        // QLabel? ?¬κΈ?
        QSize labelSize = videoLabel->size();

        // ?΄λ―Έμ??? ?¬κΈ?
        QSize imageSize = image.size();
        // ?΄λ―Έμ????? QLabel? μ’ν‘λΉ? λΉκ΅
        QRect targetRect;
        if (static_cast<float>(imageSize.width()) / imageSize.height() > static_cast<float>(labelSize.width()) / labelSize.height()) {
            // ?΄λ―Έμ??κ°? κ°?λ‘λ‘ ? κΈΈλ©΄ μ’μ°λ₯? ??Ό?
            int newWidth = static_cast<int>(imageSize.height() * static_cast<float>(labelSize.width()) / labelSize.height());
            int xOffset = (imageSize.width() - newWidth) / 2;
            targetRect = QRect(xOffset, 0, newWidth, imageSize.height());
        } else {
            // ?΄λ―Έμ??κ°? ?Έλ‘λ‘ ? κΈΈλ©΄ ???λ₯? ??Ό?
            int newHeight = static_cast<int>(imageSize.width() * static_cast<float>(labelSize.height()) / labelSize.width());
            int yOffset = (imageSize.height() - newHeight) / 2;
            targetRect = QRect(0, yOffset, imageSize.width(), newHeight);
        }

        // ?΄λ―Έμ??λ₯? ??Ό?
        QImage croppedImage = image.copy(targetRect);
        QPixmap pixmap =  QPixmap::fromImage(croppedImage).scaled(labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        emit signal_video_start();
        videoLabel->setPixmap(pixmap);
      //  qDebug() << "QPixmap created for stream_ui object at address: " << this;
           // qDebug() << "Video data address: " << pixmap;

    }

}

//void rtpClient::recv_url(QString url)
//{
//    streaming_url = url;
//}

void rtpClient::startFFmpegProcess(QString url) {
    ffmpegProcess = new QProcess();
//    #if 1 // window?κ²?
//    // FFmpeg ?€? κ²½λ‘ λ°? λͺλ Ή?΄ ?€? 
//    QString program = QDir::currentPath() + "/bin/ffmpeg.exe";
//    qDebug() << program;
//    QStringList arguments;
//    #else // linux?κ²?
    //QString program = "/usr/bin/ffmpeg";
    QString program = QDir::currentPath() + "/bin/ffmpeg.exe";

    qDebug() << program;
    QStringList arguments;
    //#endif

    arguments << "-protocol_whitelist" << "file,tcp,udp,rtp,rtsp"
              << "-i" << url // "rtsp://192.168.1.15:8554"
              << "-s" << "640x480"
              << "-pix_fmt" << "rgb24"  // ?½??? ?¬λ§·μ raw RGBλ‘? ?€? 
            //  <<"-threads"<<"1"
//              << "-b:v" << "40K"        // λΉλ?€ λΉνΈ? ?΄?Έ ?€?  (2Mbps)
//              << "-maxrate" << "40K"    // μ΅λ?? λΉνΈ? ?΄?Έ ?€? 
//?              << "-bufsize" << "4M"    // λ²νΌ ?¬κΈ? ?€?  (4Mbps)
              << "-f" << "rawvideo"    // μΆλ ₯? raw λΉλ?€λ‘? ?€? 
           //   << "-loglevel" << "debug"
              << "-";                  // stdout?Όλ‘? μΆλ ₯

    //  FFmpeg ?€?
    ffmpegProcess->start(program, arguments);
    // ??¬ μ²λ¦¬ ? ?Έ ?°κ²?
    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray errorOutput = ffmpegProcess->readAllStandardError();
        if (!errorOutput.isEmpty()) {
            if(errorOutput.contains("failed")||errorOutput.contains("No such file")||errorOutput.contains("not found")||errorOutput.contains("Invalid argument")||errorOutput.contains("Bad Request"))
            {
                emit signal_stream_fail();
                emit signal_ffmpeg_debug("FFmpeg error output:"+errorOutput,this);
            }
            emit signal_ffmpeg_debug("FFmpeg error output:"+errorOutput,this);
            qDebug()<<"ffmepg debug : "<<errorOutput;
        }
    });

    if (!ffmpegProcess->waitForStarted()) {
        qDebug() << "FFmpeg ?€? ?€?¨: " << ffmpegProcess->errorString();
    } else {
        qDebug() << "FFmpeg ?€?Έλ¦¬λ° ?? μ€?...";
        emit signal_streaming_start();
        QObject::connect(ffmpegProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readFFmpegOutput()));
    }

}
void rtpClient::finishFfmpeg()
{
    if(ffmpegProcess)
    {qDebug()<<"finishFFmpge";
        ffmpegProcess->terminate();
        if(!ffmpegProcess->waitForFinished(5000))
        {
            ffmpegProcess->kill();
        }
    }
}
