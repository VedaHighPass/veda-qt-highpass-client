#include "mailwidget.h"
#include <QTcpSocket>
#include <QLabel>

MailWidget::MailWidget(QWidget *parent)
    : QWidget(parent), subjectEdit(new QTextEdit(this)), bodyEdit(new QTextEdit(this)), sendButton(new QPushButton("Send", this)) {

    this->setWindowTitle("Compose Email");
    this->setFixedSize(400, 380); // 팝업 창 크기 설정

    QVBoxLayout *layout = new QVBoxLayout(this);

    // 제목 입력 영역
    QLabel *subjectLabel = new QLabel("Subject:", this);
    layout->addWidget(subjectLabel);
    layout->addWidget(subjectEdit);
    subjectEdit->setFixedHeight(40);

    // 본문 입력 영역
    QLabel *bodyLabel = new QLabel("Body:", this);
    layout->addWidget(bodyLabel);
    layout->addWidget(bodyEdit);
    bodyEdit->setFixedHeight(200);

    // 전송 버튼
    layout->addWidget(sendButton);

    // 버튼 클릭 시 슬롯 연결
    connect(sendButton, &QPushButton::clicked, this, &MailWidget::onSendButtonClicked);

    // 팝업 창으로 설정
    this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
}

void MailWidget::onSendButtonClicked() {
    QString subject = subjectEdit->toPlainText();
    QString body = bodyEdit->toPlainText();
    //emit sendMail(subject, body); // 메일 전송 신호 발생
    sendEmail("3katie@naver.com", "hi", "hihi");
    //this->close(); // 창 닫기
}

void MailWidget::sendEmail(const QString& recipientEmail, const QString& subject, const QString& messageBody) {
    QTcpSocket* socket = new QTcpSocket();
    socket->connectToHost("smtp.example.com", 25); // SMTP 서버와 포트

    connect(socket, &QTcpSocket::connected, [socket, recipientEmail, subject, messageBody]() {
        socket->write("EHLO localhost\r\n");
        socket->write("AUTH LOGIN\r\n");
        socket->write(QByteArray("\r\n")); // USERNAME BASE64
        socket->write(QByteArray("\r\n")); // PASSWORD BASE64
        socket->write(QString("MAIL FROM:<sender@example.com>\r\n").toUtf8());
        socket->write(QString("RCPT TO:<%1>\r\n").arg(recipientEmail).toUtf8());
        socket->write("DATA\r\n");
        socket->write(QString("Subject: %1\r\n\r\n%2\r\n.\r\n").arg(subject, messageBody).toUtf8());
        socket->write("QUIT\r\n");
    });

    connect(socket, &QTcpSocket::readyRead, [socket]() {
        qDebug() << socket->readAll(); // SMTP 서버의 응답을 읽음
    });

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}
