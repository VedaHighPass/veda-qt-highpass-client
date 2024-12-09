/*
  Copyright (c) 2011 - Tőkés Attila

  This file is part of SmtpClient for Qt.

  SmtpClient for Qt is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  SmtpClient for Qt is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the LICENCE file for more details.
*/

#ifndef SENDEMAIL_H
#define SENDEMAIL_H

#include <QWidget>

#include "external/SmtpMime/include/SmtpMime"

namespace Ui {
class SendEmail;
}

struct Client {
    QString plateNumber;
    QString email;
    QString dueAmount; // 청구 금액
    QPixmap image;
};

class SendEmail : public QWidget
{
    Q_OBJECT

public:
    explicit SendEmail(QWidget *parent = 0);
    ~SendEmail();

    static EmailAddress stringToEmail(const QString & str);
    void setRecipientEmails(const QStringList &emails);
    void setClients(const QList<Client> &clients); // Client 리스트 설정

private slots:
    void on_addAttachment_clicked();
    void on_sendEmail_clicked();

private:
    Ui::SendEmail *ui;

    void errorMessage(const QString & message);
    QStringList recipientEmails;
    QString processTemplate(const QString &templateStr, const Client &client);
    QList<Client> clients; // Client 데이터 저장
};

#endif // SENDEMAIL_H
