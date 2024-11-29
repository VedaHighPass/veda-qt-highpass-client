#include "checkboxdelegate.h"

#include <QApplication>
#include <QCheckBox>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{
}

void CheckBoxDelegate::paint( QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index ) const
{
    bool value = index.data().toBool(); // 현재 셀의 데이터를 불리언값으로 가져옴
    QStyleOptionButton cbo; // 버튼 스타일 설정

    cbo.rect = option.rect;
    cbo.rect.setLeft( option.rect.center().x() - 7 );
    cbo.state = value ? QStyle::State_On : QStyle::State_Off;
    cbo.text = "";
    cbo.state = cbo.state | QStyle::State_Enabled;

    qApp->style()->drawControl( QStyle::CE_CheckBox,&cbo, painter ); // 체크박스 컨트롤을 그리도록 지정
}

bool CheckBoxDelegate::editorEvent( QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index )
{
    Q_UNUSED(option);

    if( event->type() == QEvent::MouseButtonRelease )
        return false;

    model->setData( index, !index.data().toBool() );

    return event->type() == QEvent::MouseButtonDblClick ? true : false;
}
