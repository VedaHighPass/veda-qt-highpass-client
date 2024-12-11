#include "registeremail.h"
#include "ui_registeremail.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QGraphicsDropShadowEffect>
#include <highpasswindow.h>


RegisterEmail::RegisterEmail(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterEmail)
{
    ui->setupUi(this);

    // shadow effect
    QList<QWidget*> widgets = {
        ui->pushButton,
        ui->top_background
    };

    for (QWidget* widget : widgets) {
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
        shadowEffect->setBlurRadius(50);  // 그림자 블러 정도
        shadowEffect->setOffset(1, 1);    // 그림자 위치
        shadowEffect->setColor(QColor(0, 0, 0, 50));
        widget->setGraphicsEffect(shadowEffect);
    }
}

RegisterEmail::~RegisterEmail()
{
    delete ui;
}

void RegisterEmail::on_pushButton_clicked()
{
    // 데이터 읽기
    QString plateNumber = ui->line_platenum->text().trimmed();
    QString email = ui->line_email->text().trimmed();

    // 입력값 검증
    if (plateNumber.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Both Plate Number and Email are required.");
        return;
    }

    // JSON 생성
    QJsonObject json;
    json["PlateNumber"] = plateNumber;
    json["Email"] = email;

    // 시그널로 JSON 데이터 전송
    emit sendEmailData(json);
}
