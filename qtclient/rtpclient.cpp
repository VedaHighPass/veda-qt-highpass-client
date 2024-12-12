#include "rtpclient.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QDir>
#include <QPainter>

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
           finishFfmpeg(); // ?��?��?�� 종료 ?���?
       }
       delete ffmpegProcess;
   }
    qDebug()<<"!!!rtpCLient finished!!!!";
}

/*
void rtpClient::readFFmpegOutput() {
    if (ffmpegProcess->state() == QProcess::Running) {
        qDebug() << "FFmpeg process is not running.";
    }

    QByteArray data = ffmpegProcess->readAllStandardOutput();
    // qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    if (data.isEmpty()) {
      //  qDebug() << "No data from ffmpeg. Check the process and arguments.";
    }
   // qDebug() << "Received data size:" << data.size();
    buffer->append(data);
  //  qDebug() << "Received buffer size:" << buffer.size();
    if (buffer->size() >= 640 * 480 * 3) {
         QByteArray frameData = buffer->left(640 * 480 * 3);
          buffer->remove(0, 640 * 480 * 3);
        // qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        QImage image(reinterpret_cast<const uchar*>(frameData.data()), 640, 480, QImage::Format_RGB888);

        QSize labelSize = videoLabel->size();

        QSize imageSize = image.size();

        QRect targetRect;
        if (static_cast<float>(imageSize.width()) / imageSize.height() > static_cast<float>(labelSize.width()) / labelSize.height()) {
            int newWidth = static_cast<int>(imageSize.height() * static_cast<float>(labelSize.width()) / labelSize.height());
            int xOffset = (imageSize.width() - newWidth) / 2;
            targetRect = QRect(xOffset, 0, newWidth, imageSize.height());
        } else {
            int newHeight = static_cast<int>(imageSize.width() * static_cast<float>(labelSize.height()) / labelSize.width());
            int yOffset = (imageSize.height() - newHeight) / 2;
            targetRect = QRect(0, yOffset, imageSize.width(), newHeight);
        }

        //QImage croppedImage = image.copy(targetRect);
        QPixmap pixmap =  QPixmap::fromImage(croppedImage).scaled(labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        emit signal_video_start();
        videoLabel->setPixmap(pixmap);
        //  qDebug() << "QPixmap created for stream_ui object at address: " << this;
        // qDebug() << "Video data address: " << pixmap;
    }
}
*/

void rtpClient::readFFmpegOutput() {
    static QSize cachedLabelSize; // �� ũ�� ĳ��
    static QPixmap cachedFinalPixmap; // ���� ���� QPixmap
    static bool sizeChanged = true; // �� ũ�� ���� ����

    if (videoLabel->size() != cachedLabelSize) {
        // �� ũ�Ⱑ ����Ǿ��� ���� ���� ��� ����
        cachedLabelSize = videoLabel->size();
        sizeChanged = true;

        // ������ ����� QPixmap ����
        cachedFinalPixmap = QPixmap(cachedLabelSize);
        cachedFinalPixmap.fill(Qt::black);
    }

    // FFmpeg ������ �б�
    QByteArray data = ffmpegProcess->readAllStandardOutput();
    if (data.isEmpty()) {
        return;
    }

    buffer->append(data);
    //qDebug() << "Buffer size:" << buffer->size();
    //qDebug() << "Frame size required:" << 640 * 480 * 3;

    if (buffer->size() >= 640 * 480 * 3) {
        QByteArray frameData = buffer->left(640 * 480 * 3);
        buffer->remove(0, 640 * 480 * 3);

        QImage image(reinterpret_cast<const uchar*>(frameData.data()), 640, 480, QImage::Format_RGB888);

        if (!image.isNull()) {
            // �̹��� ũ�⸦ �� ũ�⿡ �°� ����
            QPixmap scaledPixmap = QPixmap::fromImage(image).scaled(cachedLabelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            QPixmap finalPixmap = cachedFinalPixmap; // ���� ��� ����
            QPainter painter(&finalPixmap);

            // ���� ��� ���� ������ �̹����� �߾ӿ� �׸���
            painter.drawPixmap((cachedLabelSize.width() - scaledPixmap.width()) / 2,
                               (cachedLabelSize.height() - scaledPixmap.height()) / 2,
                               scaledPixmap);

            // �󺧿� �ֽ� �̹����� ����
            videoLabel->setPixmap(finalPixmap);
            sizeChanged = false; // ũ�� ���� ���� �ʱ�ȭ
        } else {
            qDebug() << "Failed to create QImage";
        }

        emit signal_video_start();
    }
}

//void rtpClient::recv_url(QString url)
//{
//    streaming_url = url;
//}

void rtpClient::startFFmpegProcess(QString url) {
    ffmpegProcess = new QProcess();
    #if _WIN32 // window
    QString program = QDir::currentPath() + "/bin/ffmpeg.exe";
    qDebug() << program;
    QStringList arguments;
    #else // linux
    QString program = "/usr/bin/ffmpeg";
    qDebug() << program;
    QStringList arguments;
    #endif

    arguments << "-protocol_whitelist" << "file,tcp,udp,rtp,rtsp"
            //<< "-hwaccel" << "cuda"                            // �ϵ���� ���� (CUDA)
              << "-i" << url // "rtsp://192.168.1.15:8554"
              << "-s" << "640x480"
              << "-vsync" << "vfr" // ���� ����ȭ
              << "-framerate" << "30" // 30fps�� ����
              << "-pix_fmt" << "rgb24"
            //<<"-threads"<<"1"
              << "-b:v" << "2M"
              << "-maxrate" << "4M"
              << "-bufsize" << "4M"
              << "-f" << "rawvideo"
            //<< "-loglevel" << "debug"
              << "-";                  // stdout

    //  FFmpeg
    ffmpegProcess->start(program, arguments);

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
        //qDebug() << "FFmpeg : " << ffmpegProcess->errorString();
    } else {
        //qDebug() << "FFmpeg ...";
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
