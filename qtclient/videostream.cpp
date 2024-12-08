#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QString>
#include <QMenu>
#include <QTabWidget>
#include <QMdiSubWindow>
#include <QPushButton>

#include "httpclient.h"
#include "videostream.h"
#include "ui_videostream.h"
#include "rtpclient.h"
#include "stream_ui.h"


videoStream::videoStream(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::videoStream)
{
    ui->setupUi(this);
    ui->tabWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);
    httpCli = new HttpClient(this);
    httpCli->loadCameras();

    connect(httpCli,SIGNAL(signal_finishload()),this,SLOT(addRegistedCam()));
    connect(ui->tabWidget_2, &QTabWidget::customContextMenuRequested, this, &videoStream::showContextMenu);
}

QString getCurrentFormattedTime() {
    // 현재 시간을 가져옵니다.
    QDateTime currentTime = QDateTime::currentDateTime();\
    // 지정된 형식으로 시간을 포맷팅합니다.
    QString formattedTime = currentTime.toString("yyyy_MM_dd_HH:mm:ss.zz");
    return formattedTime;
}


videoStream::~videoStream()
{
    delete ui;
}

void videoStream::slot_ffmpeg_debug(QString error,rtpClient* textedit_key)
{
   if(map_textedit.contains(textedit_key))
   {
       map_textedit[textedit_key]->append(error);
   }
   //qDebug()<<error;
   // ui->textEdit->append(error);
}

void videoStream:: showContextMenu(const QPoint& pos) {
    QMenu menu(this);
    QAction* addTabAction = menu.addAction("Add Tab");
    connect(addTabAction, &QAction::triggered, this, &videoStream::addNewTab);
    menu.exec(ui->tabWidget_2->mapToGlobal(pos));

}

 void videoStream:: addNewTab() {
     stream_ui* newTab = new stream_ui(this,httpCli);
       newTab->setWindowTitle(QString("CAM %1").arg(ui->mdiArea->subWindowList().lastIndexOf(ui->mdiArea->subWindowList().last()) + 2));
       ui->mdiArea->addSubWindow(newTab);
       newTab->show();

       // 이후 최소 크기 제한 해제
       //setMinimumSize(0, 0);
       // Create the QWidget for the tab content (this will be added to the tab widget)
       QWidget* tabContentWidget = new QWidget();  // This widget will hold the QTextEdit and buttons
       QVBoxLayout* layout = new QVBoxLayout(tabContentWidget);

       // Create the QTextEdit widget
       QTextEdit* newDebug = new QTextEdit();
       newDebug->setMinimumHeight(100); // Adjust height to make space for the buttons
       layout->addWidget(newDebug); // Add QTextEdit to the layout

       // Create the button layout (horizontal)
       QWidget* buttonWidget = new QWidget();
       QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);

       // Create 3 buttons and add them to the horizontal layout
       QPushButton* button1 = new QPushButton("Pause");
       QPushButton* button2 = new QPushButton("Restart");
       QPushButton* button3 = new QPushButton("Disconnect");

       buttonLayout->addWidget(button1);
       buttonLayout->addWidget(button2);
       buttonLayout->addWidget(button3);
       connect(button1,SIGNAL(clicked()),newTab,SLOT(slot_push_pause()));
       connect(button2,SIGNAL(clicked()),newTab,SLOT(slot_push_restart()));
       connect(button3,SIGNAL(clicked()),newTab,SLOT(slot_push_disconnect()));
       // Add the button widget to the main layout
       layout->addWidget(buttonWidget);
       // Add this tab content widget (with QTextEdit and buttons) to the tab widget
       ui->tabWidget->addTab(tabContentWidget, QString("%1").arg(newTab->windowTitle()));

       // Optionally, add the new tab to the debug map for tracking
       map_textedit.insert(newTab->rtpCli, newDebug);
       map_stream_ui.insert(newTab, tabContentWidget);

       connect(newTab->rtpCli, SIGNAL(signal_ffmpeg_debug(QString, rtpClient*)), this, SLOT(slot_ffmpeg_debug(QString, rtpClient*)));
       connect(newTab, SIGNAL(signal_stream_ui_del(stream_ui*)), this, SLOT(slot_tab_del(stream_ui*)));

       qDebug() << "Created new tab with stream_ui object at address: " << newTab;
}
 void videoStream::slot_tab_del(stream_ui* delIndex)
 {

     //delIndex->rtpCli->finishFfmpeg();
     // Ensure the corresponding QTextEdit is deleted
     if (map_stream_ui.contains(delIndex)) {
         QWidget* del_widget = map_stream_ui[delIndex];
         del_widget->layout()->deleteLater();
         map_stream_ui[delIndex]->deleteLater();  // Delay the deletion of QTextEdit
         map_stream_ui.remove(delIndex);  // Remove from map
     }
     if (map_textedit.contains(delIndex->rtpCli)) {
         map_textedit[delIndex->rtpCli]->deleteLater();  // Delay the deletion of the debug QTextEdit
         map_textedit.remove(delIndex->rtpCli);  // Remove from map
     }
     QMdiSubWindow* targetSubWindow = nullptr;
     for (QMdiSubWindow* subWindow : ui->mdiArea->subWindowList()) {
         if (subWindow->widget() == delIndex) { // 위젯 주소 비교
             targetSubWindow = subWindow;
             break;
         }
     }

     // 찾은 경우에만 삭제
     if (targetSubWindow) {
         ui->mdiArea->removeSubWindow(targetSubWindow);
         targetSubWindow->deleteLater();
     } else {
         qDebug() << "Error: delIndex not found in QMdiArea.";
     }
     // Optionally remove the stream_ui object from the UI
    delIndex->deleteLater();
 }

 void videoStream:: addRegistedCam() {
     map_url = httpCli->getCameraUrlMap();
     if (map_url.isEmpty()) {
         qDebug() << "map_url is empty!";
     } else {
         qDebug() << "map_url contains data!";
     }

     for(QMap<QString, QString>::iterator it = map_url.begin();  it != map_url.end();it++){
         qDebug()<<"CAM_NAME: "<<it.key();
         qDebug()<<"CAM_URL: "<<it.value();
     stream_ui* newTab = new stream_ui(this,httpCli,it.value());
       newTab->setWindowTitle(it.key());
       ui->mdiArea->addSubWindow(newTab);
       newTab->show();

       // 이후 최소 크기 제한 해제
       //setMinimumSize(0, 0);
       // Create the QWidget for the tab content (this will be added to the tab widget)
       QWidget* tabContentWidget = new QWidget();  // This widget will hold the QTextEdit and buttons
       QVBoxLayout* layout = new QVBoxLayout(tabContentWidget);

       // Create the QTextEdit widget
       QTextEdit* newDebug = new QTextEdit();
       newDebug->setMinimumHeight(100); // Adjust height to make space for the buttons
       layout->addWidget(newDebug); // Add QTextEdit to the layout

       // Create the button layout (horizontal)
       QWidget* buttonWidget = new QWidget();
       QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);

       // Create 3 buttons and add them to the horizontal layout
       QPushButton* button1 = new QPushButton("Pause");
       QPushButton* button2 = new QPushButton("Restart");
       QPushButton* button3 = new QPushButton("Disconnect");
       connect(button1,SIGNAL(clicked()),newTab,SLOT(slot_push_pause()));
       connect(button2,SIGNAL(clicked()),newTab,SLOT(slot_push_restart()));
       connect(button3,SIGNAL(clicked()),newTab,SLOT(slot_push_disconnect()));

       buttonLayout->addWidget(button1);
       buttonLayout->addWidget(button2);
       buttonLayout->addWidget(button3);

       // Add the button widget to the main layout
       layout->addWidget(buttonWidget);
       // Add this tab content widget (with QTextEdit and buttons) to the tab widget
       ui->tabWidget->addTab(tabContentWidget, QString("%1").arg(newTab->windowTitle()));

       // Optionally, add the new tab to the debug map for tracking
       map_textedit.insert(newTab->rtpCli, newDebug);
       map_stream_ui.insert(newTab, tabContentWidget);

       connect(newTab->rtpCli, SIGNAL(signal_ffmpeg_debug(QString, rtpClient*)), this, SLOT(slot_ffmpeg_debug(QString, rtpClient*)));
       connect(newTab, SIGNAL(signal_stream_ui_del(stream_ui*)), this, SLOT(slot_tab_del(stream_ui*)));
       //connect(newTab, SIGNAL(signal_stream_ui_del(stream_ui*)), this, SLOT(deletSubWind(stream_ui*)));

       qDebug() << "Created new tab with stream_ui object at address: " << newTab;
       connect(this,SIGNAL(start_stream()),newTab,SLOT(on_startBtn_clicked()));
       emit start_stream();
    }
}
