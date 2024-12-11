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
           finishFfmpeg(); // ?ïà?†Ñ?ïú Ï¢ÖÎ£å ?ò∏Ï∂?
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
    // ?ç∞?ù¥?Ñ∞?ùò ?Å¨Í∏? ?ôï?ù∏
   // qDebug() << "Received data size:" << data.size();
    buffer->append(data);
  //  qDebug() << "Received buffer size:" << buffer.size();
    // raw RGB ?ç∞?ù¥?Ñ∞Î°? Í∞??†ï?ïòÍ≥? QImageÎ°? Î≥??ôò
    if (buffer->size() >= 640 * 480 * 3) {  // ?ï¥?ÉÅ?èÑ 640x480?óê ????ï¥ RGB?äî 3Î∞îÏù¥?ä∏ ?îΩ???
         QByteArray frameData = buffer->left(640 * 480 * 3);
          buffer->remove(0, 640 * 480 * 3);
        // qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        QImage image(reinterpret_cast<const uchar*>(frameData.data()), 640, 480, QImage::Format_RGB888);

        // QLabel?ùò ?Å¨Í∏?
        QSize labelSize = videoLabel->size();

        // ?ù¥ÎØ∏Ï???ùò ?Å¨Í∏?
        QSize imageSize = image.size();
        // ?ù¥ÎØ∏Ï????? QLabel?ùò Ï¢ÖÌö°Îπ? ÎπÑÍµê
        QRect targetRect;
        if (static_cast<float>(imageSize.width()) / imageSize.height() > static_cast<float>(labelSize.width()) / labelSize.height()) {
            // ?ù¥ÎØ∏Ï??Í∞? Í∞?Î°úÎ°ú ?çî Í∏∏Î©¥ Ï¢åÏö∞Î•? ?ûò?ùº?ÉÑ
            int newWidth = static_cast<int>(imageSize.height() * static_cast<float>(labelSize.width()) / labelSize.height());
            int xOffset = (imageSize.width() - newWidth) / 2;
            targetRect = QRect(xOffset, 0, newWidth, imageSize.height());
        } else {
            // ?ù¥ÎØ∏Ï??Í∞? ?Ñ∏Î°úÎ°ú ?çî Í∏∏Î©¥ ?úÑ?ïÑ?ûòÎ•? ?ûò?ùº?ÉÑ
            int newHeight = static_cast<int>(imageSize.width() * static_cast<float>(labelSize.height()) / labelSize.width());
            int yOffset = (imageSize.height() - newHeight) / 2;
            targetRect = QRect(0, yOffset, imageSize.width(), newHeight);
        }

        // ?ù¥ÎØ∏Ï??Î•? ?ûò?ùº?ÉÑ
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
    #if _WIN32 // window?ôòÍ≤?
    // FFmpeg ?ã§?ñâ Í≤ΩÎ°ú Î∞? Î™ÖÎ†π?ñ¥ ?Ñ§?†ï
    QString program = QDir::currentPath() + "/bin/ffmpeg.exe";
    qDebug() << program;
    QStringList arguments;
    #else // linux?ôòÍ≤?
    QString program = "/usr/bin/ffmpeg";
    qDebug() << program;
    QStringList arguments;
    #endif



    arguments << "-protocol_whitelist" << "file,tcp,udp,rtp,rtsp"
              << "-i" << url // "rtsp://192.168.1.15:8554"
              << "-s" << "640x480"
              << "-pix_fmt" << "rgb24"  // ?îΩ??? ?è¨Îß∑ÏùÑ raw RGBÎ°? ?Ñ§?†ï
            //  <<"-threads"<<"1"
//              << "-b:v" << "40K"        // ÎπÑÎîî?ò§ ÎπÑÌä∏?†à?ù¥?ä∏ ?Ñ§?†ï (2Mbps)
//              << "-maxrate" << "40K"    // ÏµúÎ?? ÎπÑÌä∏?†à?ù¥?ä∏ ?Ñ§?†ï
//?Öã              << "-bufsize" << "4M"    // Î≤ÑÌçº ?Å¨Í∏? ?Ñ§?†ï (4Mbps)
              << "-f" << "rawvideo"    // Ï∂úÎ†•?ùÑ raw ÎπÑÎîî?ò§Î°? ?Ñ§?†ï
           //   << "-loglevel" << "debug"
              << "-";                  // stdout?úºÎ°? Ï∂úÎ†•

    //  FFmpeg ?ã§?ñâ
    ffmpegProcess->start(program, arguments);
    // ?óê?ü¨ Ï≤òÎ¶¨ ?ã†?ò∏ ?ó∞Í≤?
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
        qDebug() << "FFmpeg ?ã§?ñâ ?ã§?å®: " << ffmpegProcess->errorString();
    } else {
        qDebug() << "FFmpeg ?ä§?ä∏Î¶¨Î∞ç ?ãú?ûë Ï§?...";
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
