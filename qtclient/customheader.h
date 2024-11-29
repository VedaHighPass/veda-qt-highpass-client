#include <QHeaderView>
#include <QStyleOptionButton>
#include <QPainter>
#include <QMouseEvent>

class CustomHeader : public QHeaderView
{
    Q_OBJECT

public:
    explicit CustomHeader(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent), isChecked(false) {}

signals:
    void headerCheckboxToggled(bool checked); // 체크박스 상태 변경 신호

protected:
    // 헤더를 그릴 때 체크박스를 추가
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override {
        QHeaderView::paintSection(painter, rect, logicalIndex);

        if (logicalIndex == 0) { // 첫 번째 열에 체크박스 그리기
            QStyleOptionButton option;
            option.rect = QRect(rect.left() + 5, rect.top() + 5, 16, 16); // 체크박스 위치 조정
            option.state = QStyle::State_Enabled | (isChecked ? QStyle::State_On : QStyle::State_Off);
            style()->drawControl(QStyle::CE_CheckBox, &option, painter);
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        int logicalIndex = logicalIndexAt(event->pos());
        if (logicalIndex == 0) { // 첫 번째 열 헤더 클릭
            int headerXPosition = sectionViewportPosition(0); // 첫 번째 열의 뷰포트 x 좌표
            QRect checkboxRect(headerXPosition + 5, 5, 16, 16); // 체크박스 영역 설정
            if (checkboxRect.contains(event->pos())) {
                isChecked = !isChecked; // 체크 상태 토글
                emit headerCheckboxToggled(isChecked); // 신호 방출
                viewport()->update(); // 헤더 업데이트
                return; // 기본 헤더 클릭 처리 중단
            }
        }
        QHeaderView::mousePressEvent(event); // 기본 동작 처리
    }

private:
    bool isChecked; // 체크박스 상태
};
