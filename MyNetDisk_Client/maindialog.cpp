#include "maindialog.h"
#include "ui_maindialog.h"

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("我的网盘");
    //Qt::FramelessWindowHint无边框
    //this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                         Qt::WindowCloseButtonHint);
    //添加右键菜单
    /*
     With Qt::CustomContextMenu,
     the signal customContextMenuRequested() is emitted
    */
    ui->table_file->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->table_file , SIGNAL(customContextMenuRequested(QPoint))
            ,this, SLOT(slot_menuShow(QPoint)));
    m_menu.addAction(QIcon(":/images/folder.png"),"新建文件夹");
    m_menu.addAction("下载");
    m_menu.addAction("删除");
    m_menu.addAction("收藏");
    m_menu.addAction("分享");
    m_menu.addAction("获取分享");

    //triggered 触发
    connect(&m_menu , SIGNAL(triggered(QAction*)) ,
            this,SLOT(slot_dealMenu(QAction*)));

    m_menuAddFile.addAction("上传文件");
    m_menuAddFile.addAction("上传文件夹");
    connect(&m_menuAddFile , SIGNAL(triggered(QAction*)) ,
            this,SLOT(slot_dealMenuAddFile(QAction*)));

    ui->sw_right->setCurrentIndex( 0 );//默认显示文件
    ui->tw_transmit->setCurrentIndex( 1 );//传输里面默认下载分页
    //添加上传下载  右键菜单
    ui->table_upload->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->table_upload , SIGNAL(customContextMenuRequested(QPoint))
            ,this, SLOT(slot_menuUploadShow(QPoint)));

    ui->table_download->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->table_download , SIGNAL(customContextMenuRequested(QPoint))
            ,this, SLOT(slot_menuDownloadShow(QPoint)));

    m_menuUpload.addAction("暂停");
    m_menuUpload.addAction("开始");
    connect(&m_menuUpload , SIGNAL(triggered(QAction*)) ,
            this,SLOT(slot_dealMenuUpload(QAction*)));

    m_menuDownload.addAction("暂停");
    m_menuDownload.addAction("开始");
    connect(&m_menuDownload , SIGNAL(triggered(QAction*)) ,
            this,SLOT(slot_dealMenuDownload(QAction*)));

}
/*
 最小化、最大化、关闭
WindowMinimizeButtonHint | WindowMaximizeButtonHint |
WindowCloseButtonHint
*/

MainDialog::~MainDialog()
{
    delete ui;
}
#include <QMessageBox>
void MainDialog::closeEvent(QCloseEvent *event)
{
    if(QMessageBox::question(this ,"退出提示", "是否退出？")
            == QMessageBox::Yes)
    {
        event->accept();//事件执行,关闭窗口
        Q_EMIT SIG_close();
    }else{
        event->ignore();//事件忽略
    }
}


void MainDialog::on_pb_filePage_clicked()
{
    ui->sw_right->setCurrentIndex(0);
}


void MainDialog::on_pb_transmitPage_clicked()
{
    ui->sw_right->setCurrentIndex(1);
}


void MainDialog::on_pb_sharePage_clicked()
{
    ui->sw_right->setCurrentIndex(2);
}

void MainDialog::slot_setInfo(QString name)
{
    ui->pb_user->setText(name);
}

void MainDialog::slot_insertFileInfo(FileInfo &info)
{
    //插入假数据测试
    MyTableWidgeItem * item0 = new MyTableWidgeItem;
    item0->setInfo(info);


    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(info.time);
    QTableWidgetItem * item2 = new QTableWidgetItem;
    if(info.type != "dir")
        item2->setText(FileInfo::getSize(info.size));
    else
        item2->setText("");

    int row = ui->table_file->rowCount();//新增1行
    ui->table_file->setRowCount(row + 1);

    ui->table_file->setItem(row, 0 , item0);
    ui->table_file->setItem(row, 1 , item1);
    ui->table_file->setItem(row, 2 , item2);
}
#include <QProgressBar>
void MainDialog::slot_insertDownloadFile(FileInfo &info)
{
    int row = ui->table_download->rowCount();//新增1行
    ui->table_download->setRowCount(row + 1);
    //插入
    MyTableWidgeItem * item0 = new MyTableWidgeItem;
    item0->setInfo(info);


    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QProgressBar * progress = new QProgressBar;//窗口一旦回收，其他所有子控件会自动回收
    progress->setMaximum(info.size);
    progress->setValue(0);

    QPushButton * pButton = new QPushButton;
    if(info.isPause == 0)
        pButton->setText("暂停");
    else
        pButton->setText("开始");

    ui->table_download->setItem(row, 0 , item0);
    ui->table_download->setItem(row, 1 , item1);
    ui->table_download->setCellWidget(row, 2 , progress);
    ui->table_download->setCellWidget(row, 3 , pButton);

}

void MainDialog::slot_insertUploadFile(FileInfo &info)
{
    int row = ui->table_upload->rowCount();//新增1行
    ui->table_upload->setRowCount(row + 1);
    //插入
    MyTableWidgeItem * item0 = new MyTableWidgeItem;
    item0->setInfo(info);


    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QProgressBar * progress = new QProgressBar;//窗口一旦回收，其他所有子控件会自动回收
    progress->setMaximum(info.size);
    progress->setValue(0);

    QPushButton * pButton = new QPushButton;
    if(info.isPause == 0)
        pButton->setText("暂停");
    else
        pButton->setText("开始");

    ui->table_upload->setItem(row, 0 , item0);
    ui->table_upload->setItem(row, 1 , item1);
    ui->table_upload->setCellWidget(row, 2 , progress);
    ui->table_upload->setCellWidget(row, 3 , pButton);
}
#include <QDateTime>
void MainDialog::slot_insertCompleteFile(FileInfo &info)
{
    int row = ui->table_complete->rowCount();//新增1行
    ui->table_complete->setRowCount(row + 1);
    //插入
    MyTableWidgeItem * item0 = new MyTableWidgeItem;
    item0->setInfo(info);


    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QTableWidgetItem * item2 = new QTableWidgetItem;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    item2->setText(time);

    QPushButton* pButton = new QPushButton;
    pButton->setIcon(QIcon(":/images/folder.png"));
    pButton->setFlat( true );//设置按钮扁平化
    //tooltip
    pButton->setToolTip(info.absolutePath);
    connect(pButton , SIGNAL(clicked()) ,
            this, SLOT(slot_folderButtonClick()));


    ui->table_complete->setItem(row, 0 , item0);
    ui->table_complete->setItem(row, 1 , item1);
    ui->table_complete->setItem(row, 2 , item2);
    ui->table_complete->setCellWidget(row , 3 ,pButton);

}

void MainDialog::slot_insertShareFile(QString name, QString size, QString time, QString link)
{
    int row = ui->table_share->rowCount();//新增1行
    ui->table_share->setRowCount(row + 1);
    //插入

    QTableWidgetItem * item0 = new QTableWidgetItem;
    item0->setText(name);

    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(size);

    QTableWidgetItem * item2 = new QTableWidgetItem;
    item2->setText(time);

    QTableWidgetItem * item3 = new QTableWidgetItem;
    item3->setText(link);





    ui->table_share->setItem(row, 0 , item0);
    ui->table_share->setItem(row, 1 , item1);
    ui->table_share->setItem(row, 2 , item2);
    ui->table_share->setItem(row, 3 , item3);
}

void MainDialog::slot_insertUploadCompleteFile(FileInfo &info)
{
    int row = ui->table_complete->rowCount();//新增1行
    ui->table_complete->setRowCount(row + 1);
    //插入
    MyTableWidgeItem * item0 = new MyTableWidgeItem;
    item0->setInfo(info);


    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QTableWidgetItem * item2 = new QTableWidgetItem;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    item2->setText(time);

    QPushButton* pButton = new QPushButton;
    pButton->setText("上传完成");
    pButton->setFlat( true );//设置按钮扁平化
    //tooltip
    pButton->setToolTip(info.absolutePath);



    ui->table_complete->setItem(row, 0 , item0);
    ui->table_complete->setItem(row, 1 , item1);
    ui->table_complete->setItem(row, 2 , item2);
    ui->table_complete->setCellWidget(row , 3 ,pButton);
}
#include <QDebug>
void MainDialog::slot_updateFileProgress(int fileid, int pos)
{
    qDebug()<< __func__ << fileid << pos;

    //遍历 table  取0列  取fileid 判断是否相等 更新 第二列
    int row = ui->table_download->rowCount();
    for(int i = 0;i < row;++i){
        MyTableWidgeItem * item0 =
                (MyTableWidgeItem *)ui->table_download->item(i,0);
        if(item0->m_info.fileid == fileid){
            QProgressBar * progress =
                    (QProgressBar *)ui->table_download->cellWidget(i , 2 );
            progress->setValue(pos);

            if(progress->value() >= progress->maximum()){
                //插入已完成的信息
                this->slot_insertCompleteFile( item0->m_info );
                //删除该条
                ui->table_download->removeRow(i);
            }
            return;
        }

    }
}

void MainDialog::slot_updateUploadFileProgress(int fileid, int pos)
{
    qDebug()<< __func__ << fileid << pos;

    //遍历 table  取0列  取fileid 判断是否相等 更新 第二列
    int row = ui->table_upload->rowCount();
    for(int i = 0;i < row;++i){
        MyTableWidgeItem * item0 =
                (MyTableWidgeItem *)ui->table_upload->item(i,0);
        if(item0->m_info.fileid == fileid){
            QProgressBar * progress =
                    (QProgressBar *)ui->table_upload->cellWidget(i , 2 );
            progress->setValue(pos);

            if(progress->value() >= progress->maximum()){
                //插入已完成的信息
                this->slot_insertUploadCompleteFile( item0->m_info );
                //删除该条
                ui->table_upload->removeRow(i);
            }
            return;
        }

    }
}

void MainDialog::slot_deleteAllFileInfo()
{
    //ui->table_file->clear();//不可以，行数不会清掉

    //所以要逐行清掉
    int row = ui->table_file->rowCount();
    for(int i = row - 1; i >= 0 ; --i ){
        ui->table_file->removeRow( i );
    }
}

void MainDialog::slot_deleteAllShareFileInfo()
{
    //ui->table_file->clear();//不可以，行数不会清掉

    //所以要逐行清掉
    int row = ui->table_share->rowCount();
    for(int i = row - 1; i >= 0 ; --i ){
        ui->table_share->removeRow( i );
    }
}

void MainDialog::slot_menuShow(QPoint point)
{
    //在鼠标点击的位置显示
    m_menu.exec(QCursor::pos());
}

void MainDialog::slot_menuDownloadShow(QPoint point)
{
    //在鼠标点击的位置显示
    m_menuDownload.exec(QCursor::pos());
}

void MainDialog::slot_menuUploadShow(QPoint point)
{
    //在鼠标点击的位置显示
    m_menuUpload.exec(QCursor::pos());
}
#include "qDebug"
#include <QInputDialog>
void MainDialog::slot_dealMenu(QAction *action)
{
    if(action->text() == "新建文件夹")
    {
        qDebug() << "新建文件夹";
        //弹窗 输入 获取文本   过滤
        QString name = QInputDialog::getText( this , "新建文件夹" , "输入名称：");
        //过滤  / \\ ? " * : | <> 不允许存在
        if(name.isEmpty()){
            name = "新建文件夹";
        }
        //去重
        int row = ui->table_file->rowCount();
        for(int i = 0; i < row; ++i){
            QString txt = ui->table_file->item( i , 0 )->text();
            if( txt == name ){
                QMessageBox::about( this , "提示：" , "名字不可重复");
                return;
            }
        }
        //过滤
        QString tmp = name;
        if(tmp.remove(" ").isEmpty()){
            QMessageBox::about( this , "提示：" , "名字不可全是空格");
            return;
        }
        if(name.contains( "/" ) || name.contains( "\\" )
                || name.contains( "?" ) || name.contains( "\"" )
                || name.contains( ":" ) || name.contains( "*" )
                || name.contains( "|") || name.contains( "<" )
                || name.contains( ">" ) )
        {
            QMessageBox::about( this , "提示：" , "名字不可包含：/ \\ ? \" * : | <>");
            return;
        }

        //发信号给核心类
        Q_EMIT SIG_addFolder( name );

    }else if(action->text() == "下载"){
        qDebug() << "下载";

        //遍历表单  看哪一行是打勾的 ， 获取控件
        int rows = ui->table_file->rowCount();
        for(int i = 0; i < rows;++i){
            //看对应的文件id
            MyTableWidgeItem * item = (MyTableWidgeItem *)ui->table_file->item( i , 0);

            //判断是否被勾选
            if(item->checkState() == Qt::Checked){
                int fileid = item->m_info.fileid;
                //查看是否在当前的下载列表里 todo

                //不在

                //发一个  下载 信号
                Q_EMIT SIG_downloadFile(fileid);
            }

        }

    }else if(action->text() == "删除"){
        qDebug() << "删除";
        //遍历表单  看哪一行是打勾的 ， 获取控件
        int rows = ui->table_file->rowCount();
        QString path = ui->pb_path->text();
        QVector<int> fileidArray;
        for(int i = 0; i < rows;++i){
            //看对应的文件id
            MyTableWidgeItem * item = (MyTableWidgeItem *)ui->table_file->item( i , 0);

            //判断是否被勾选
            if(item->checkState() == Qt::Checked){
                int fileid = item->m_info.fileid;


                //发一个  下载 信号
                fileidArray.push_back( fileid );
            }

        }
        //发信号时自己写的声明需要加类型
        Q_EMIT SIG_deleteFile( path , fileidArray );
    }else if(action->text() == "收藏"){
        qDebug() << "收藏";
    }else if(action->text() == "分享"){
        qDebug() << "分享";

        //遍历表单  看哪一行是打勾的 ， 获取控件
        int rows = ui->table_file->rowCount();
        QString path = ui->pb_path->text();
        QVector<int> fileidArray;
        for(int i = 0; i < rows;++i){
            //看对应的文件id
            MyTableWidgeItem * item = (MyTableWidgeItem *)ui->table_file->item( i , 0);

            //判断是否被勾选
            if(item->checkState() == Qt::Checked){
                int fileid = item->m_info.fileid;


                //发一个  下载 信号
                fileidArray.push_back( fileid );
            }

        }
        Q_EMIT SIG_shareFile( path , fileidArray);
    }else if(action->text() == "获取分享"){
        qDebug() << "获取分享";

        //弹窗 输入分享码
        QString txt = QInputDialog::getText( this , "获取分享" , "输入分享码" );
        //过滤  分享码 xxxx xxxx
        int code = txt.toInt();
        if( code >= 100000000 || code < 10000000 || txt.length() != 8){
            QMessageBox::about( this , "提示" , "共享码非法");
            return;
        }
        QString path = ui->pb_path->text();
        //发信号 分享码  目录
        Q_EMIT SIG_getShare( path , code);
    }
}
#include <QFileDialog>
void MainDialog::slot_dealMenuAddFile(QAction *action)
{
    if(action->text() == "上传文件"){
            qDebug() << "上传文件";
            //弹窗  选择文件
            QString path = QFileDialog::getOpenFileName(this,"上传文件");
            if(path.isEmpty()){
                return;
            }
            //判断，该文件是否正在上传  正在上传 不让传todo

            //上传的文件信息 在核心类打包 存储文件信息 所以要用信号 传递
            Q_EMIT SIG_uploadFile(path);

    }else if(action->text() == "上传文件夹"){
            qDebug() << "上传文件夹";
            //弹窗  选择路径

    }
}

void MainDialog::slot_dealMenuDownload(QAction *action)
{
    if( action->text() == "暂停") {
       //判断现在的状态是否是暂停
        //遍历所有  看是否选中
        int rows = ui->table_download->rowCount();
        for(int i = 0; i < rows;++i){
            MyTableWidgeItem* item = (MyTableWidgeItem*)ui->table_download->item(i , 0);
            if(item->checkState() == Qt::Checked){
                QPushButton* button = (QPushButton*)ui->table_download->cellWidget(i , 3);
                if(button->text() == "暂停"){
                    button->setText("开始");
                    Q_EMIT SIG_setDownloadPause(item->m_info.fileid,1);
                }
            }
        }
        //判断现在的状态  文本是暂停 才操作 不然退出
            //按钮切换
            //文件状态切换----> 发送信号  槽函数改变map中文件info 对应的标志位
       qDebug() << "暂停";
    }else if(action->text() == "开始"){
        int rows = ui->table_download->rowCount();
        for(int i = 0; i < rows;++i){
            MyTableWidgeItem* item = (MyTableWidgeItem*)ui->table_download->item( i, 0);
            if(item->checkState() == Qt::Checked){
                QPushButton* button = (QPushButton*)ui->table_download->cellWidget(i , 3);
                if(button->text() == "开始"){
                    button->setText("暂停");
                    Q_EMIT SIG_setDownloadPause(item->m_info.fileid,0);
                }
            }
        }
       qDebug() << "开始";
    }
}

void MainDialog::slot_dealMenuUpload(QAction *action)
{
    if( action->text() == "暂停") {
        //遍历所有  看是否选中
        int rows = ui->table_upload->rowCount();
        for(int i = 0; i < rows;++i){
            MyTableWidgeItem* item = (MyTableWidgeItem*)ui->table_upload->item(i , 0);
            if(item->checkState() == Qt::Checked){
                QPushButton* button = (QPushButton*)ui->table_upload->cellWidget(i , 3);
                if(button->text() == "暂停"){
                    button->setText("开始");
                    Q_EMIT SIG_setUploadPause(item->m_info.fileid,1);
                }
            }
        }
        //判断现在的状态  文本是暂停 才操作 不然退出
            //按钮切换
            //文件状态切换----> 发送信号  槽函数改变map中文件info 对应的标志位
       qDebug() << "暂停";
    }else if(action->text() == "开始"){
        //遍历所有  看是否选中
        int rows = ui->table_upload->rowCount();
        for(int i = 0; i < rows;++i){
            MyTableWidgeItem* item = (MyTableWidgeItem*)ui->table_upload->item( i, 0);
            if(item->checkState() == Qt::Checked){
                QPushButton* button = (QPushButton*)ui->table_upload->cellWidget(i , 3);
                if(button->text() == "开始"){
                    button->setText("暂停");
                    Q_EMIT SIG_setUploadPause(item->m_info.fileid,0);
                }
            }
        }
        //判断现在的状态  文本是暂停 才操作 不然退出
            //按钮切换
            //文件状态切换----> 发送信号  槽函数改变map中文件info 对应的标志位
       qDebug() << "开始";
    }
}
#include <QProcess>
void MainDialog::slot_folderButtonClick()
{
    //能拿到发送信号的对象
    QPushButton* pButton = (QPushButton*)QObject::sender();
    QString path = pButton->toolTip().replace('/' , '\\');

    //错误用法：path = path.replace('/' , '\\');

    QProcess process;
    process.startDetached("explorer",
                          QStringList()<<QString("/select,")<<path);

}

FileInfo &MainDialog::slot_getDownloadFileInfoByFileID(int fileid)
{
    //遍历所有的下载信息 （第0列）

    int rows = ui->table_download->rowCount();
    for(int i = 0; i < rows;++i){
        MyTableWidgeItem* item = (MyTableWidgeItem*)ui->table_download->item( i, 0);
        if( fileid == item->m_info.fileid ){//id 相等 返回文件信息
            return item->m_info;
        }
    }
}

FileInfo &MainDialog::slot_getUploadFileInfoByFileID(int fileid)
{
    //遍历所有的下载信息 （第0列）

    int rows = ui->table_upload->rowCount();
    for(int i = 0; i < rows;++i){
        MyTableWidgeItem* item = (MyTableWidgeItem*)ui->table_upload->item( i, 0);
        if( fileid == item->m_info.fileid ){//id 相等 返回文件信息
            return item->m_info;
        }
    }
}


void MainDialog::on_table_file_cellClicked(int row, int column)
{
    QTableWidgetItem * item = ui->table_file->item(row,0);
    if(item->checkState() == Qt::Unchecked){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}


void MainDialog::on_pb_addFile_clicked()
{
    //弹出菜单
    m_menuAddFile.exec(QCursor::pos());
}

//处理双击
void MainDialog::on_table_file_cellDoubleClicked(int row, int column)
{
    //路径跳转
    ui->pb_path->text();
    //拿到双击的文本
    MyTableWidgeItem * item = (MyTableWidgeItem *)ui->table_file->item(row , 0);
    //item->text();
    if( item->m_info.type != "file"){
        QString path = ui->pb_path->text() + item->text() + "/";
        ui->pb_path->setText( path );

        slot_deleteAllFileInfo();

        Q_EMIT SIG_changeDir(path);

    }
}

//void MainDialog::on_pb_path_clicked()
//{

//}



void MainDialog::on_pb_prevDir_clicked()
{
    //ui->pb_path->text();  /pinkk/  lastIndexOf  返回  从右侧开始找第一个的下标
    QString path = ui->pb_path->text();
    if( path == "/") return;
    path = path.left(path.lastIndexOf("/"));//left + 长度
    path = path.left(path.lastIndexOf("/") + 1);  //  /
    qDebug() << path;

    //界面的应用分为前台和后台
    ui->pb_path->setText(path);
    Q_EMIT SIG_changeDir(path);

}


void MainDialog::on_table_download_cellClicked(int row, int column)
{
    QTableWidgetItem * item = ui->table_download->item(row,0);
    if(item->checkState() == Qt::Unchecked){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}


void MainDialog::on_table_upload_cellClicked(int row, int column)
{
    QTableWidgetItem * item = ui->table_upload->item(row,0);
    if(item->checkState() == Qt::Unchecked){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}

