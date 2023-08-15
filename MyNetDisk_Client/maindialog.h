#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include "common.h"
#include "mytablewidgeitem.h"
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();

    void SIG_downloadFile(int fileid);

    void SIG_uploadFile(QString path);

    void SIG_uploadFolder(QString path);

    void SIG_addFolder( QString name );

    void SIG_changeDir(QString path);

    void SIG_deleteFile( QString path , QVector<int> fileidArray );

    void SIG_shareFile( QString path , QVector<int> fileidArray );

    void SIG_getShare( QString path , int link );

    void SIG_setUploadPause( int fileid , int isPause);

    void SIG_setDownloadPause( int fileid , int isPause);

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();
//关闭事件
    void closeEvent(QCloseEvent *event);
private slots:
    void on_pb_filePage_clicked();

    void on_pb_transmitPage_clicked();

    void on_pb_sharePage_clicked();
    void on_table_file_cellClicked(int row, int column);

    void on_pb_addFile_clicked();

    void on_table_file_cellDoubleClicked(int row, int column);

    //void on_pb_path_clicked();

    void on_pb_prevDir_clicked();

    void on_table_download_cellClicked(int row, int column);

    void on_table_upload_cellClicked(int row, int column);

public slots:
    void slot_setInfo(QString name);

    void slot_insertFileInfo(FileInfo & info);

    void slot_insertDownloadFile(FileInfo &info);

    void slot_insertUploadFile(FileInfo &info);

    void slot_insertCompleteFile(FileInfo &info);

    void slot_insertShareFile( QString name,QString size,QString time,QString link);

    void slot_insertUploadCompleteFile(FileInfo &info);
    //下载的
    void slot_updateFileProgress( int fileid , int pos);
    //上传的
    void slot_updateUploadFileProgress( int fileid , int pos);
    //删除所有的文件信息
    void slot_deleteAllFileInfo();

    void slot_deleteAllShareFileInfo();

    void slot_menuShow(QPoint point);

    void slot_menuDownloadShow(QPoint point);

    void slot_menuUploadShow(QPoint point);

    void slot_dealMenu(QAction* action);

    void slot_dealMenuAddFile(QAction* action);

    void slot_dealMenuDownload(QAction* action);

    void slot_dealMenuUpload(QAction* action);

    void slot_folderButtonClick();

    FileInfo& slot_getDownloadFileInfoByFileID( int fileid );

    FileInfo& slot_getUploadFileInfoByFileID( int fileid );

private:
    Ui::MainDialog *ui;

    QMenu m_menu;
    QMenu m_menuAddFile;
    QMenu m_menuDownload;
    QMenu m_menuUpload;
};
#endif // MAINDIALOG_H
