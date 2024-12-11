#ifndef REGISTEREMAIL_H
#define REGISTEREMAIL_H

#include <QWidget>

namespace Ui {
class RegisterEmail;
}

class RegisterEmail : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterEmail(QWidget *parent = nullptr);
    ~RegisterEmail();

signals:
    void sendEmailData(const QJsonObject &json); // JSON 데이터를 전달하는 시그널

private slots:
    void on_pushButton_clicked();

private:
    Ui::RegisterEmail *ui;
};

#endif // REGISTEREMAIL_H
