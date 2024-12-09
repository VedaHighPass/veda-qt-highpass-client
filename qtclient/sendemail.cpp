/*
  Copyright (c) 2011 - Tőkés Attila

  This file is part of SmtpClient for Qt.

  SmtpClient for Qt is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  SmtpClient for Qt is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the LICENSE file for more details.
*/

#include "sendemail.h"
#include "ui_sendemail.h"

#include <QFileDialog>
#include <QErrorMessage>
#include <QMessageBox>

#include <iostream>
#include <QBuffer>

#include "demo_vars.h"

using namespace std;

SendEmail::SendEmail(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendEmail)
{
    ui->setupUi(this);

    ui->host->setText(SMTP_SERVER);
    ui->port->setValue(465);
    ui->ssl->setChecked(true);
    ui->auth->setChecked(true);
    ui->username->setText(SENDER_EMAIL);
    ui->password->setText(SENDER_PASSWORD);

    ui->sender->setText(QString(SENDER_NAME) + "<" + SENDER_EMAIL + ">");
    //ui->recipients->setText(QString(RECIPIENT_NAME) + "<" + RECIPIENT_EMAIL + ">");
}

SendEmail::~SendEmail()
{
    delete ui;
}

EmailAddress SendEmail::stringToEmail(const QString &str)
{
    int p1 = str.indexOf("<");
    int p2 = str.indexOf(">");

    if (p1 == -1)
    {
        // no name, only email address
        return EmailAddress(str);
    }
    else
    {
        return EmailAddress(str.mid(p1 + 1, p2 - p1 - 1), str.left(p1));
    }

}

void SendEmail::on_addAttachment_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec())
        ui->attachments->addItems(dialog.selectedFiles());
}

void SendEmail::on_sendEmail_clicked()
{
    QString host = ui->host->text();
    int port = ui->port->value();
    bool ssl = ui->ssl->isChecked();
    bool auth = ui->auth->isChecked();
    QString user = ui->username->text();
    QString password = ui->password->text();

    EmailAddress sender = stringToEmail(ui->sender->text());

    if (clients.isEmpty()) {
        QMessageBox::warning(this, "No Recipients", "No client data available for email sending.");
        return;
    }

    QString subjectTemplate = ui->subject->text();
    QString bodyTemplate = ui->texteditor->toHtml();

    SmtpClient smtp(host, port, ssl ? SmtpClient::SslConnection : SmtpClient::TcpConnection);

    smtp.connectToHost();
    if (!smtp.waitForReadyConnected()) {
        errorMessage("Connection Failed");
        return;
    }

    if (auth) {
        smtp.login(user, password);
        if (!smtp.waitForAuthenticated()) {
            errorMessage("Authentication Failed");
            return;
        }
    }

    for (const auto &client : clients) {
        EmailAddress recipient(client.email);

        MimeMessage message;
        message.setSender(sender);
        message.addRecipient(recipient);

        // 템플릿 치환
        QString subject = processTemplate(subjectTemplate, client);
        QString htmlBody = processTemplate(bodyTemplate, client);

        message.setSubject(subject);

        // HTML 콘텐츠 설정
        MimeHtml content;
        content.setHtml(htmlBody); // HTML 본문 설정

        message.addPart(&content);

        // add attached file
        QList<QFile*> files;
        for (int i = 0; i < ui->attachments->count(); ++i)
        {
            QFile* file = new QFile(ui->attachments->item(i)->text());
            files.append(file);

            MimeAttachment* attachment = new MimeAttachment(file);
            message.addPart(attachment, true);
        }

        QList<QString> tempFiles; // 임시 파일 경로를 저장할 리스트
        if (!client.image.isNull()) {
            // 임시 파일 생성
            QString tempPath = QDir::tempPath() + "/" + client.plateNumber + ".png";
            qDebug() << "Temporary file path:" << tempPath;
            if (client.image.save(tempPath, "PNG")) { // 이미지를 임시 파일에 저장
                QFile *file = new QFile(tempPath);
                if (file->open(QIODevice::ReadOnly)) {
                    MimeAttachment *attachment = new MimeAttachment(file); // QFile 사용
                    message.addPart(attachment, true); // 첨부 파일 추가
                    tempFiles.append(tempPath);
                } else {
                    qDebug() << "Failed to open temp file for plate number:" << client.plateNumber;
                    delete file;
                }
            } else {
                qDebug() << "Failed to save image to temp file for plate number:" << client.plateNumber;
            }
        }
        qDebug() << "Sending email to:" << client.email;
        qDebug() << "Subject:" << subject;
        qDebug() << "HTML Body:" << htmlBody;

        smtp.sendMail(message);
        if (!smtp.waitForMailSent()) {
            qDebug() << "Failed to send email to" << client.email;
        } else {
            qDebug() << "Email sent to" << client.email;
        }

        for (auto file : files) {
            delete file;
        }
        // 이메일 전송 완료 후 임시 파일 삭제
        for (const QString &tempPath : tempFiles) {
            if (QFile::exists(tempPath)) {
                QFile::remove(tempPath);
                qDebug() << "Temporary file deleted:" << tempPath;
            }
        }
    }

    smtp.quit();

    QMessageBox::information(this, "Email Sent", "Emails have been sent successfully.");
}


void SendEmail::errorMessage(const QString &message)
{
    QErrorMessage err (this);

    err.showMessage(message);

    err.exec();
}

void SendEmail::setRecipientEmails(const QStringList &emails)
{
    recipientEmails = emails;

    // 이메일 리스트를 UI recipients 필드에 추가
    QStringList emailStrings;
    for (const QString &email : recipientEmails) {
        emailStrings.append(email); // 이메일 문자열로 추가
    }

    ui->recipients->setText(emailStrings.join(";")); // 세미콜론으로 구분
}

QString SendEmail::processTemplate(const QString &templateStr, const Client &client)
{
    QString processedStr = templateStr;
    processedStr.replace("%PLATENUM", client.plateNumber); // 번호판 치환
    processedStr.replace("%DUE", client.dueAmount);       // 청구 금액 치환
    return processedStr;
}

void SendEmail::setClients(const QList<Client> &clients)
{
    this->clients = clients;

    // 이메일 필드에 클라이언트 이메일 추가
    QStringList emailStrings;
    for (const auto &client : clients) {
        emailStrings.append(client.email);
    }

    ui->recipients->setText(emailStrings.join(";")); // 세미콜론으로 구분
}
