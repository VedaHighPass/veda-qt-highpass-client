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
        return;
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
    painter->restore();
}

bool CheckBoxDelegate::editorEvent( QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index )
{
    Q_UNUSED(option);

    if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
        QVariant value = index.data(Qt::CheckStateRole);
        Qt::CheckState state = (value == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
        bool success = model->setData(index, state, Qt::CheckStateRole); // 모델 업데이트
        return true;
    }
    return false;
}
