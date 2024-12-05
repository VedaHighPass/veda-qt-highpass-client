#include "checkboxdelegate.h"

#include <QApplication>
#include <QCheckBox>
#include <QProxyStyle>
#include <QPainter>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{
}

void CheckBoxDelegate::paint( QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index ) const
{
    QVariant value = index.data(Qt::CheckStateRole); // Qt::CheckStateRole 사용
    if (!value.isValid()) {
        return; // 데이터가 없으면 아무것도 렌더링하지 않음
    }

    QRect rect = QRect(option.rect.center().x() - 8,  // 체크박스 X 위치
                       option.rect.center().y() - 8,  // 체크박스 Y 위치
                       16, 16);                      // 체크박스 크기

    QColor borderColor = Qt::black;
    QColor backgroundColor = (value.toInt() == Qt::Checked) ? QColor("lightblue") : QColor("white");

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 체크박스 배경
    painter->setBrush(backgroundColor);
    painter->setPen(borderColor);
    painter->drawRect(rect);

    // 체크 표시 추가
    if (value.toInt() == Qt::Checked) {
        painter->setPen(Qt::black);
        painter->drawLine(rect.topLeft() + QPoint(4, 8), rect.center() + QPoint(0, 4)); // 첫 번째 선
        painter->drawLine(rect.center() + QPoint(0, 4), rect.bottomRight() - QPoint(2, 10)); // 두 번째 선
    }

    // 기본 체크박스 내부 그리기
    //QApplication::style()->drawControl(QStyle::CE_CheckBox, &cbo, painter);

    painter->restore();
}

bool CheckBoxDelegate::editorEvent( QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index )
{
    qDebug() << "Editor event called for row:" << index.row() << "col:" << index.column();
    qDebug() << "Event type:" << event->type(); // 이벤트 타입 확인
    Q_UNUSED(option);

    if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
        qDebug() << "Mouse event detected. Current CheckState:" << index.data(Qt::CheckStateRole);
        QVariant value = index.data(Qt::CheckStateRole);
        Qt::CheckState state = (value == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
        qDebug() << "Setting CheckState to:" << state;
        bool success = model->setData(index, state, Qt::CheckStateRole); // 모델 업데이트
        qDebug() << "Model setData success:" << success;
        return true;
    }
    return false;
}
