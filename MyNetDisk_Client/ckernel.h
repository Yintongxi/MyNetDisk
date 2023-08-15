#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include <iostream>
#include "maindialog.h"
#include "packdef.h"
#include "logindialog.h"
#include "csqlite.h"


using namespace std;
class INetMediator;
//单例  1.私有化： 构造函数、拷贝构造函数、析构函数 2.提供一个静态的公有获取对象的方法
class CKernel;
//类函数指针类型
typedef void (CKernel::*PFUN)( unsigned int lSendIP , char* buf , int nlen );

class CKernel : public QObject
{
    Q_OBJECT

private:
    explicit CKernel(QObject *parent = nullptr);
    //explicit 防止隐式类型转换，莫名奇妙的创建一个对象
    /*当类的声明和定义分别在两个文件中时，explicit只能写在在声明中，不能写在定义中*/
    ~CKernel(){}
    CKernel(const CKernel& kernel){}

private:
    //ui类
    MainDialog* m_mainDialog;
    LoginDialog* m_loginDialog;
    QString m_ip;
    int m_port;
    INetMediator * m_tcpClient;
    //INetMediator * m_tcpServer;

    QString m_name;
    int m_id;
    QString m_curDir;
    //协议映射数组
    PFUN m_netPackMap[_DEF_PROTOCOL_COUNT];
public:
    void setConfig();
    void setNetMap();

public:
    static CKernel* getInstance(){
        //最简单写法
        static CKernel kernel;
        return &kernel;
    }

signals:
    void SIG_updateFileProgress( int fileid , int pos);
    void SIG_updateUploadFileProgress( int fileid , int pos);
private slots:
//处理控件信号
    void slot_destroyInstance();
    void slot_registerCommit(QString tel,QString passwd,QString name);
    void slot_loginCommit(QString tel,QString passwd);
    void slot_downloadFile(int fileid);
    void slot_uploadFile(QString path);
    void slot_uploadFolder(QString path);
    void slot_updateFileList();
    void slot_addFolder( QString name );
    void slot_changeDir(QString path);
    void slot_deleteFile( QString path , QVector<int> fileidArray );
    void slot_shareFile( QString path , QVector<int> fileidArray );
    void slot_refreshMyShare();
    void slot_getShare(QString path , int code);
    void slot_setUploadPause( int fileid , int isPause);
    void slot_setDownloadPause(int fileid , int isPause);


//网络处理
    void slot_clientReadyData( unsigned int lSendIP , char* buf , int nlen );
    void slot_serverReadyData( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileHeadRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileContentRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealUploadFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileContentRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealAddFolderRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealQuickUploadRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealDeleteFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealShareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealMyShareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGetShareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealContinueUploadRs( unsigned int lSendIP , char* buf , int nlen );


//处理注册回复
    void slot_dealRegisterRs( unsigned int lSendIP , char* buf , int nlen );
//处理登录回复
    void slot_dealLoginRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileInfo( unsigned int lSendIP , char* buf , int nlen );
private:
    void SendData(char* buf , int nlen);

    void initDatabase(int id);

    void slot_writeUploadTask(FileInfo& info);

    void slot_writeDownloadTask(FileInfo& info);

    void slot_deleteUploadTask(FileInfo& info);

    void slot_deleteDownloadTask(FileInfo& info);

    void slot_getUploadTask(QList<FileInfo>& infoList);

    void slot_getDownloadTask(QList<FileInfo>& infoList);



    std::map<int , FileInfo> m_mapFileidToFileInfo;
    std::map<std::string , FileInfo> m_mapFileMD5ToFileInfo;

    QString m_sysPath;
    bool m_quit;

    CSqlite * m_sql;
};

#endif // CKERNEL_H
