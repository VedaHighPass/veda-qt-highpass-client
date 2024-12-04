#ifndef MAILWIDGET_H
#define MAILWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

class MailWidget : public QWidget {
    Q_OBJECT

public:
    explicit MailWidget(QWidget *parent = nullptr);

signals:
    void sendMail(const QString &subject, const QString &body);

private slots:
    void onSendButtonClicked();

private:
    QTextEdit *subjectEdit;
    QTextEdit *bodyEdit;
    QPushButton *sendButton;
    void sendEmail(const QString& recipientEmail, const QString& subject, const QString& messageBody);
};


#endif // MAILWIDGET_H
