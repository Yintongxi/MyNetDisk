#include "ckernel.h"
#include "QDebug"

#include "TcpClientMediator.h"
#include "TcpServerMediator.h"
#include<QCoreApplication>
#include <QMessageBox>
#include <QThread>
#include "md5.h"


#include <iostream>
using namespace std;

#define MD5_KEY "1234"
string getMD5(QString val)
{
    QString str = QString("%1_%2").arg(val).arg(MD5_KEY);
    MD5 md(str.toStdString());
    qDebug() << str << "MD5: " << md.toString().c_str();
    return md.toString();
}
void Utf8ToGB2312( char* gbbuf , int nlen ,QString& utf8);

string getFileMD5(QString path){
    //打开文件 将文件都读入到 md5对象里
    char buf[1000] = "";

    FILE * pFile = NULL;
    Utf8ToGB2312(buf,1000,path);
    pFile = fopen(buf,"rb");
    if(!pFile){
        qDebug()<<"md5 open file fail...";
        return string();
    }
    int len = 0;
    MD5 md;
    do{
        len = fread(buf, 1 ,1000,pFile);     
        md.update(buf,len);//不断滴拼字符串并求取md5
        QCoreApplication::processEvents( QEventLoop::AllEvents , 100 );//解决大文件在while中执行时间长卡顿问题
    }while(len > 0);
    fclose(pFile);
    //然后就生成了 md5
    qDebug()<<"MD5:"<<md.toString().c_str();
    return md.toString();
}



#include<QTextCodec>

// QString -> char* gb2312
void Utf8ToGB2312( char* gbbuf , int nlen ,QString& utf8)
{
    //转码的对象
    QTextCodec * gb2312code = QTextCodec::codecForName( "gb2312");
    //QByteArray char 类型数组的封装类 里面有很多关于转码 和 写IO的操作
    QByteArray ba = gb2312code->fromUnicode( utf8 );// Unicode -> 转码对象的字符集

    strcpy_s ( gbbuf , nlen , ba.data() );
}

// char* gb2312 --> QString utf8
QString GB2312ToUtf8( char* gbbuf )
{
    //转码的对象
    QTextCodec * gb2312code = QTextCodec::codecForName( "gb2312");
    //QByteArray char 类型数组的封装类 里面有很多关于转码 和 写IO的操作
    return gb2312code->toUnicode( gbbuf );// 转码对象的字符集 -> Unicode
}
CKernel::CKernel(QObject *parent) : QObject(parent)
  ,m_id(0),m_curDir("/"),m_quit(false)
{
    setConfig();
    setNetMap();

    m_sql = new CSqlite;

    m_tcpClient = new TcpClientMediator;

    m_tcpClient->OpenNet( m_ip.toStdString().c_str() , m_port);

    connect(m_tcpClient , SIGNAL(SIG_ReadyData(uint,char*,int)) ,
            this , SLOT(slot_clientReadyData(uint,char*,int)));

//    STRU_LOGIN_RQ rq;
//    m_tcpClient->SendData(0, (char*)&rq , sizeof(rq));

    m_mainDialog = new MainDialog;
    //m_mainDialog->show();

    connect(m_mainDialog, SIGNAL(SIG_close()) ,
            this , SLOT(slot_destroyInstance()));
    connect(m_mainDialog, SIGNAL(SIG_downloadFile(int)),
            this,SLOT(slot_downloadFile(int)));
    connect(this, SIGNAL(SIG_updateFileProgress(int,int)),
            m_mainDialog,SLOT(slot_updateFileProgress(int,int)));
    connect(m_mainDialog, SIGNAL(SIG_uploadFile(QString)),
            this,SLOT(slot_uploadFile(QString)));
    connect(m_mainDialog, SIGNAL(SIG_uploadFolder(QString)),
            this,SLOT(slot_uploadFolder(QString)));
    connect(this,SIGNAL(SIG_updateUploadFileProgress(int,int)),
            m_mainDialog,SLOT(slot_updateUploadFileProgress(int,int)));
    connect(m_mainDialog,SIGNAL(SIG_addFolder(QString)),
            this,SLOT(slot_addFolder(QString)));
    connect(m_mainDialog,SIGNAL(SIG_changeDir(QString)),
            this,SLOT(slot_changeDir(QString)));
    connect(m_mainDialog,SIGNAL(SIG_deleteFile(QString,QVector<int>)),
            this,SLOT(slot_deleteFile(QString,QVector<int>)));
    connect(m_mainDialog,SIGNAL(SIG_shareFile(QString,QVector<int>)),
            this,SLOT(slot_shareFile(QString,QVector<int>)));
    connect(m_mainDialog,SIGNAL(SIG_getShare(QString,int)),
            this,SLOT(slot_getShare(QString,int)));
    connect(m_mainDialog,SIGNAL(SIG_setUploadPause(int,int)),
            this,SLOT(slot_setUploadPause(int,int)));
    connect(m_mainDialog,SIGNAL(SIG_setDownloadPause(int,int)),
            this,SLOT(slot_setDownloadPause(int,int)));




    m_loginDialog = new LoginDialog;
    connect(m_loginDialog,SIGNAL(SIG_close()),
            this,SLOT(slot_destroyInstance()));
    connect(m_loginDialog,SIGNAL(SIG_registerCommit(QString,QString,QString)),
            this,SLOT(slot_registerCommit(QString,QString,QString)));
    connect(m_loginDialog,SIGNAL(SIG_loginCommit(QString,QString)),
            this,SLOT(slot_loginCommit(QString,QString)));
    m_loginDialog->show();

}
#include <QCoreApplication>
#include <QSettings>//配置文件的类
#include <QFileInfo>
#include <QDir>
//设置配置文件
void CKernel::setConfig()
{//windows下   *.ini  --> config.ini
 /*
    [net] 组名
    key = value 键值对
    ip = "202.118.199.203"
    port = 8004
    配置文件保存在.exe同级下
 */
 //获取 exe目录
    //D:/myProcess/Netdisk/build-MyNetDisk/debug
    QString path = QCoreApplication::applicationDirPath()+"/config.ini";
    m_ip = "192.168.144.130";
    m_port = 8003;

    //考虑当前目录下，是否有配置文件
    QFileInfo info(path);
    if(info.exists()){//有，读取
        QSettings settings(path,QSettings::IniFormat,nullptr);
        settings.beginGroup("net");
        QVariant strip = settings.value("ip" , "");
        if(!strip.toString().isEmpty()) m_ip = strip.toString();
        QVariant nport = settings.value("port" , 0);
        if( nport.toInt() != 0) m_port = nport.toInt();
        settings.endGroup();
    }else{//目录中没有配置文件，创建
        QSettings settings(path,QSettings::IniFormat,nullptr);
        settings.beginGroup("net");  //先访问组，再到key - value
        settings.setValue("ip" , m_ip);
        settings.setValue("port" , m_port);
        settings.endGroup();
    }
    qDebug() << "ip:" << m_ip << "port:" << m_port;

    //查看是否有默认路径QCoreApplication::applicationDirPath()
    QString sysPath = QCoreApplication::applicationDirPath()+"/NetDisk/";
    QDir dir;
    if(!dir.exists(sysPath))
        dir.mkdir(sysPath);
    //fileinfo  dir  带/ 的 要拼接
    m_sysPath = QCoreApplication::applicationDirPath()+"/NetDisk";
}

#define NetMap(a) m_netPackMap[a - _DEF_PROTOCOL_BASE]
void CKernel::setNetMap()
{
    memset(m_netPackMap,0,sizeof(PFUN) * _DEF_PROTOCOL_COUNT);
    NetMap(_DEF_PACK_REGISTER_RS) = &CKernel::slot_dealRegisterRs;
    NetMap(_DEF_PACK_LOGIN_RS) = &CKernel::slot_dealLoginRs;
    NetMap(_DEF_PACK_FILE_INFO) = &CKernel::slot_dealFileInfo;
    NetMap(_DEF_PACK_FILE_HEAD_RQ) = &CKernel::slot_dealFileHeadRq;
    NetMap(_DEF_PACK_FILE_CONTENT_RQ) = &CKernel::slot_dealFileContentRq;
    NetMap(_DEF_PACK_UPLOAD_FILE_RS) = &CKernel::slot_dealUploadFileRs;
    NetMap(_DEF_PACK_FILE_CONTENT_RS) = &CKernel::slot_dealFileContentRs;
    NetMap(_DEF_PACK_ADD_FOLDER_RS) = &CKernel::slot_dealAddFolderRs;
    NetMap(_DEF_PACK_QUICK_UPLOAD_RS) = &CKernel::slot_dealQuickUploadRs;
    NetMap(_DEF_PACK_DELETE_FILE_RS) = &CKernel::slot_dealDeleteFileRs;
    NetMap(_DEF_PACK_SHARE_FILE_RS) = &CKernel::slot_dealShareFileRs;
    NetMap(_DEF_PACK_MY_SHARE_RS) = &CKernel::slot_dealMyShareFileRs;
    NetMap(_DEF_PACK_GET_SHARE_RS) = &CKernel::slot_dealGetShareFileRs;
    NetMap(_DEF_PACK_CONTINUE_UPLOAD_RS) = &CKernel::slot_dealContinueUploadRs;






}

void CKernel::slot_destroyInstance()
{
    qDebug() << __func__ ;
    //m_mainDialog->hide();//是窗口隐藏，不是关闭，因为再执行关闭会再次进入信号槽，死循环

    //结束暂停的while循环
    m_quit = true;

    //回收网络
    m_tcpClient->CloseNet();
    //m_tcpServer->CloseNet();
    delete m_tcpClient;
    //delete m_tcpServer;

    delete m_mainDialog;//主窗口的回收
    delete m_loginDialog;//登录窗口的回收
}
//注册提交槽函数
void CKernel::slot_registerCommit(QString tel, QString passwd, QString name)
{
    STRU_REGISTER_RQ rq;
    //QString->char* 包括兼容中文
    std::string telStr = tel.toStdString();
    strcpy(rq.tel ,telStr.c_str());

    //不希望别人拦截数据包后，获取到密码；将明文变成密文 MD5值
    //MD5 信息摘要算法第5版（信息一致性和完整性的验证）
    //12_1234
    //A8491DA37739E16168F6387E6BA76C69
    std::string passStr = getMD5(passwd)/*passwd.toStdString()*/;
    strcpy(rq.password ,passStr.c_str());

    std::string nameStr = name.toStdString();
    strcpy(rq.name ,nameStr.c_str());

    SendData((char*)&rq , sizeof(rq));

}
//登陆提交槽函数
void CKernel::slot_loginCommit(QString tel, QString passwd)
{
    STRU_LOGIN_RQ rq;
    //QString->char* 包括兼容中文
    std::string telStr = tel.toStdString();
    strcpy(rq.tel ,telStr.c_str());
    std::string passStr = getMD5(passwd); /*passwd.toStdString()*/
    strcpy(rq.password ,passStr.c_str());


    SendData((char*)&rq , sizeof(rq));
}

void CKernel::slot_downloadFile(int fileid)
{
    //打包 发送
    STRU_DOWNLOAD_RQ rq;
    rq.userid = m_id;
    rq.fileid = fileid;
    std::string strDir = m_curDir.toStdString();
    strcpy(rq.dir , strDir.c_str());

    SendData((char*)&rq,sizeof(rq));
}
#include<QFileInfo>
#include<QDateTime>
#include <iostream>
void CKernel::slot_uploadFile(QString path)
{
    QFileInfo fileInfo(path);
    //fileInfo.fileName();// 1.txt
    //fileInfo.size(); //大小
    //创建文件信息结构体
    FileInfo info;
    info.absolutePath = path;
    info.dir = m_curDir;
    //info.fileid =
    info.md5 = QString::fromStdString(getFileMD5(path));
    info.name = fileInfo.fileName();

    info.size = fileInfo.size();
    info.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    info.type = "file";
    //打开文件
    char pathBuf[1000] = "";
    Utf8ToGB2312(pathBuf,1000,path);
    info.pFile = fopen(pathBuf,"rb");
    //求解文件md5   getFileMD5()

    //添加map 里面 map[md5] = fileInfo;

    m_mapFileMD5ToFileInfo[info.md5.toStdString()] = info;
    //上传文件  打包
    STRU_UPLOAD_FILE_RQ rq;

    std::string strDir = info.dir.toStdString();
    strcpy(rq.dir , strDir.c_str());
    std::cout << "dir" << rq.dir <<std::endl;

    //有中文的话，要这么转；没中文直接转
    std::string strName = info.name.toStdString();
    strcpy(rq.fileName , strName.c_str());

    strcpy(rq.fileType,"file");
    strcpy(rq.md5,info.md5.toStdString().c_str());

    rq.size = info.size;
    strcpy(rq.time,info.time.toStdString().c_str());
    rq.userid = m_id;

    //发送
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_uploadFolder(QString path)
{

}

void CKernel::slot_updateFileList()
{
    //删除所有的列表项
    m_mainDialog->slot_deleteAllFileInfo();
    //获取用户根目录'/'下的所有文件
    STRU_FILE_LIST_RQ rq;
    rq.userid = m_id;
    std::string curDir = m_curDir.toStdString();
    strcpy(rq.dir,curDir.c_str());
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_addFolder(QString name)
{
    //打包发送

    //上传文件  打包
    STRU_ADD_FOLDER_RQ rq;

    std::string strDir = m_curDir.toStdString();
    strcpy(rq.dir , strDir.c_str());
    std::cout << "dir" << rq.dir <<std::endl;

    //有中文的话，要这么转；没中文直接转
    std::string strName = name.toStdString();
    strcpy(rq.fileName , strName.c_str());

    strcpy(rq.fileType,"dir");


    rq.size = 0;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.time,time.toStdString().c_str());
    rq.userid = m_id;

    //发送
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_changeDir(QString path)
{
    cout << "slot_changeDir"<<endl;
    m_curDir = path;
    slot_updateFileList();
}

void CKernel::slot_deleteFile(QString path, QVector<int> fileidArray)
{
    int len = sizeof(STRU_DELETE_FILE_RQ) + sizeof(int) * fileidArray.size();
    STRU_DELETE_FILE_RQ * rq = (STRU_DELETE_FILE_RQ *)malloc(len);
    rq->init();
    rq->userid = m_id;
    rq->fileCount = fileidArray.size();
    //路径考虑到有中文可能，要先转一下
    string strPath = path.toStdString();
    strcpy(rq->dir,strPath.c_str());
    for(int i = 0; i < fileidArray.size() ; ++i){
        rq->fileidArray[i] = fileidArray[i];
    }

    SendData( (char *) rq , len);
    free(rq);

}

void CKernel::slot_shareFile(QString path, QVector<int> fileidArray)
{
    //提交 请求
    int len = sizeof(STRU_SHARE_FILE_RQ) + fileidArray.size()* sizeof(int);
    STRU_SHARE_FILE_RQ * rq = (STRU_SHARE_FILE_RQ *)malloc(len);
    rq->init();
    //兼容中文，要先转成string
    string strDir = path.toStdString();
    strcpy( rq->dir , strDir.c_str() );

    rq->itemCount = fileidArray.size();

    //QDateTime -> QString
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq->shareTime , time.toStdString().c_str());

    rq->userid = m_id;
    for(int i = 0; i < fileidArray.size() ; ++i){
        rq->fileidArray[i] = fileidArray[i];
    }

    SendData( (char*)rq , len);


    free(rq);
}

void CKernel::slot_refreshMyShare()
{
    STRU_MY_SHARE_RQ rq;
    rq.userid = m_id;

    SendData( (char*)&rq , sizeof(rq));
}

void CKernel::slot_getShare(QString path, int code)
{
    //根据提交信息  发包
    STRU_GET_SHARE_RQ rq;
    rq.shareLink = code;
    rq.userid = m_id;
    //兼容中文，先转为string
    string strPath = path.toStdString();
    strcpy( rq.dir , strPath.c_str());

    SendData((char*)&rq , sizeof(rq));
}

void CKernel::slot_setUploadPause(int fileid, int isPause)
{
    if( m_mapFileidToFileInfo.count(fileid) > 0){
    //map  fileid 正常有  走暂停回复  可以暂停和恢复
        m_mapFileidToFileInfo[fileid].isPause = isPause;

    }else{
    //map  fileid 没有  断点续传:把上传的信息写入数据库，程序启动登录后加载到上传列表，并且状态为 pause 只可以继续
        if(isPause == 0){
            //断点续传 todo 从哪里开始   看服务器
            //如果路径下面没有  就直接移除
            FileInfo fileInfo = m_mainDialog->slot_getUploadFileInfoByFileID( fileid );
            //转换为 ANSI
            char pathbuf[1000] = "";
            Utf8ToGB2312(pathbuf,1000,fileInfo.absolutePath);
            //打开文件
            fileInfo.pFile = fopen(pathbuf,"rb");//二进制只读的形式打开
            if( !fileInfo.pFile ){
                qDebug() << "文件打开失败" << fileInfo.absolutePath;
                return;
            }
            //避免继续之后马上卡在那里
            fileInfo.isPause = 0;
            //info 放入id的map里
            m_mapFileidToFileInfo[fileInfo.fileid] = fileInfo;

            //续传协议 通信
            STRU_CONTINUE_UPLOAD_RQ rq;
            rq.userid = m_id;
            rq.fileid = fileInfo.fileid;

            string strDir = fileInfo.dir.toStdString();
            strcpy( rq.dir , strDir.c_str() );
            //发续传 请求
            SendData( (char*)&rq , sizeof(rq));




        }
    }
}

void CKernel::slot_setDownloadPause(int fileid, int isPause)
{
    if( m_mapFileidToFileInfo.count(fileid) > 0){
    //map  fileid 正常有  走暂停回复  可以暂停和恢复
        m_mapFileidToFileInfo[fileid].isPause = isPause;

    }else{
    //map  fileid 没有  断点续传:把上传的信息写入数据库，程序启动登录后加载到上传列表，并且状态为 pause 只可以"继续"
        if(isPause == 0){
            //断点续传 todo 从哪里开始看 客户端
            //续传其实相当于传文件头 对齐
            //下载 从哪里开始写 客户端知道   服务端不知道
            //服务器 map中是否有这个项 分为有（还没有删掉---客户端异常  迅速回复）和没有（退出后 过了很久 隔了很多天登录）

            //创建info  来自于控件
            FileInfo fileInfo = m_mainDialog->slot_getDownloadFileInfoByFileID( fileid );



            //转换为 ANSI
            char pathbuf[1000] = "";
            Utf8ToGB2312(pathbuf,1000,fileInfo.absolutePath);
            //打开文件
            fileInfo.pFile = fopen(pathbuf,"ab");//a追加  文件流在末尾不用跳转
            if( !fileInfo.pFile ){
                qDebug() << "文件打开失败" << fileInfo.absolutePath;
                return;
            }


            //避免继续之后马上卡在那里
            fileInfo.isPause = 0;
            //加入到map
            m_mapFileidToFileInfo[fileInfo.fileid] = fileInfo;

            //续传协议 通信
            STRU_CONTINUE_DOWNLOAD_RQ rq;
            rq.userid = m_id;
            rq.fileid = fileInfo.fileid;
            rq.pos = fileInfo.pos;
            string strDir = fileInfo.dir.toStdString();
            strcpy( rq.dir , strDir.c_str() );

            SendData( (char*)&rq , sizeof(rq));
        }
    }
}

void CKernel::slot_serverReadyData(unsigned int lSendIP, char *buf, int nlen)
{
//    std::string text = buf;
//    QMessageBox::about(nullptr , "服务器接收",
//                       QString("来自客户端:%1").arg(QString::fromStdString(text)));
//    //回显
//    m_tcpServer->SendData(lSendIP,buf,nlen);
    //    delete[] buf;
}
#include <QDebug>
void CKernel::slot_dealFileHeadRq(unsigned int lSendIP, char *buf, int nlen)
{

    //拆包
    STRU_FILE_HEAD_RQ* rq = (STRU_FILE_HEAD_RQ*)buf;

    qDebug() << "slot_dealFileHeadRq" << rq->fileid<< m_id <<endl;
    //创建 info
    FileInfo fileInfo;
    fileInfo.dir = QString::fromStdString(rq->dir);
    fileInfo.fileid = rq->fileid;
    fileInfo.name = QString::fromStdString(rq->fileName);
    fileInfo.type = QString::fromStdString(rq->fileType);

    fileInfo.md5 = QString::fromStdString(rq->md5);
    fileInfo.size = rq->size;
    //绝对路径  默认路径： exe同级下面 /NetDisk/dir/name
    fileInfo.absolutePath = m_sysPath + fileInfo.dir + fileInfo.name;
    //循环创建目录
    QDir dir;  // /0913/1/2/3/
    QStringList substr = fileInfo.dir.split( "/" );
    QString pathsum = m_sysPath + "/";
    for(int i = 0; i < substr.size() ; ++i){
        if(((QString)(substr.at(i))).isEmpty()) continue;
            pathsum += substr.at(i) + "/";

        if( !dir.exists(pathsum)){
            dir.mkdir(pathsum);
        }
    }

    //转换为 ANSI
    char pathbuf[1000] = "";
    Utf8ToGB2312(pathbuf,1000,fileInfo.absolutePath);
    //打开文件
    fileInfo.pFile = fopen(pathbuf,"wb");//文件打开是以二进制写的方式打开的
    if( !fileInfo.pFile ){
        qDebug() << "文件打开失败" << fileInfo.absolutePath;
        return;
    }

    slot_writeDownloadTask( fileInfo );

    m_mainDialog->slot_insertDownloadFile(fileInfo);
    //加入到map
    m_mapFileidToFileInfo[fileInfo.fileid] = fileInfo;

    //写文件头回复
    STRU_FILE_HEAD_RS rs;
    rs.fileid = rq->fileid;
    rs.userid = m_id;
    rs.result = 1;//为0应该是文件打开失败

    qDebug() << "slot_dealFileHeadRq" << rs.fileid<< rs.userid <<endl;
    SendData((char*)&rs,sizeof(rs));

}

void CKernel::slot_dealFileContentRq(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_FILE_CONTENT_RQ* rq = (STRU_FILE_CONTENT_RQ*)buf;
    rq->fileid;

    if(m_mapFileidToFileInfo.count(rq->fileid) == 0)return;

    //写到文件里
    FileInfo& info = m_mapFileidToFileInfo[rq->fileid];
    while(info.isPause){
        //sleep();
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents , 100);
        if(m_quit) return;
    }
    int len = fwrite(rq->content,1 ,rq->len , info.pFile);
    //1 2一次写多少 3写多少次 4返回成功的次数

    //有可能失败
    //写文件内容回复
    STRU_FILE_CONTENT_RS rs;
    if(len != rq->len){
        //失败 ->回跳
        fseek(info.pFile , -1*len ,SEEK_CUR);
        rs.result = 0;
        //返回失败
    }else{
        info.pos += len;
        rs.result = 1;
        //发送更新文件进度  不把更新进度函数写里面，因为会占用IO读写，主事件可能未响应
        Q_EMIT SIG_updateFileProgress( info.fileid , info.pos);
    }
    rs.len = rq->len;//失败  回跳的长度
    rs.fileid = rq->fileid;
    rs.userid = rq->userid;



    if(info.pos >= info.size){

        fclose(info.pFile);
        slot_deleteDownloadTask( info );
        m_mapFileidToFileInfo.erase(info.fileid);
    }

    SendData((char*)&rs,sizeof(rs));

}

void CKernel::slot_dealUploadFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    cout << "slot_dealUploadFileRs" <<endl;
    //拆包  获取到信息
    STRU_UPLOAD_FILE_RS * rs = (STRU_UPLOAD_FILE_RS *)buf;


    //拿文件信息
    //map md5 -> id map
    if( !m_mapFileMD5ToFileInfo.count( rs->md5 )) return;//在取之前一定要看下有没有
    FileInfo & info = m_mapFileMD5ToFileInfo[ rs->md5 ];
    //将文件id  写入文件信息
    info.fileid = rs->fileid;

    m_mapFileidToFileInfo[ rs->fileid ] = info; //将md5中的关系  拿到 fileid 对应的文件信息关系

    //插入信息
    slot_writeUploadTask( info );
    m_mainDialog->slot_insertUploadFile( info );

    STRU_FILE_CONTENT_RQ rq;
    //读文件
    int len = fread( rq.content , 1 , _DEF_BUFFER , info.pFile );
    rq.len = len;
    rq.fileid = rs->fileid;
    rq.userid = rs->userid;

    //发 文件内容
    SendData( (char * )&rq , sizeof(rq) );
    //从md5  map 删除 该点
    m_mapFileMD5ToFileInfo.erase( rs->md5 );

}

void CKernel::slot_dealFileContentRs(unsigned int lSendIP, char *buf, int nlen)
{
    printf("slot_dealFileContentRs");
    //拆包  获取到信息
    STRU_FILE_CONTENT_RS * rs = (STRU_FILE_CONTENT_RS *)buf;


    //拿文件信息
    //map md5 -> id map
    if( !m_mapFileidToFileInfo.count( rs->fileid )) return;//在取之前一定要看下有没有
    FileInfo & info = m_mapFileidToFileInfo[ rs->fileid ];

    while(info.isPause){
        //sleep();
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents , 100);
        if(m_quit) return;
    }

    //判断成功失败
    if(rs->result == 0){
        //失败
        fseek( info.pFile , -(rs->len) , SEEK_CUR );
    }else{
        info.pos += rs->len;

        //更新进度
        Q_EMIT SIG_updateUploadFileProgress(/*info.fileid*/rs->fileid , info.pos);
        //判断是否文件末尾
        if( info.pos >= info.size ){
            //判断
            if(info.dir == m_curDir){
                slot_updateFileList();
            }

            //关闭文件 回收
            fclose( info.pFile );
            slot_deleteUploadTask( info );
            m_mapFileidToFileInfo.erase( rs->fileid );
            return;
        }
    }

    STRU_FILE_CONTENT_RQ rq;
    //读文件
    int len = fread( rq.content , 1 , _DEF_BUFFER , info.pFile );
    rq.len = len;
    rq.fileid = rs->fileid;
    rq.userid = rs->userid;

    //发 文件内容
    SendData( (char * )&rq , sizeof(rq) );

}

void CKernel::slot_dealAddFolderRs(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_ADD_FOLDER_RS * rs = (STRU_ADD_FOLDER_RS *)buf;
    //判断成功  更新列表
    if(rs->result == 1){
        slot_updateFileList();
    }

}
//处理快传回复
void CKernel::slot_dealQuickUploadRs(unsigned int lSendIP, char *buf, int nlen)
{
    cout << __func__ <<endl;
    //拆包
    STRU_QUICK_UPLOAD_RS * rs = (STRU_QUICK_UPLOAD_RS *)buf;
    cout<<rs->result<<endl;
    if(rs->result == 0)return;
    //rs->md5;
    if(m_mapFileMD5ToFileInfo.count(rs->md5) == 0) return;
    FileInfo& info = m_mapFileMD5ToFileInfo[ rs->md5 ];

    //更新列表
    slot_updateFileList();
    //插入已完成的信息
    m_mainDialog->slot_insertCompleteFile( info );


    m_mapFileMD5ToFileInfo.erase( rs->md5 );

}

void CKernel::slot_dealDeleteFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_DELETE_FILE_RS * rs = (STRU_DELETE_FILE_RS *)buf;
    if(rs->result == 0) return;
    if(QString::fromStdString(rs->dir) == m_curDir){

        //刷新列表
        slot_updateFileList();

    }
}

void CKernel::slot_dealShareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_SHARE_FILE_RS * rs = (STRU_SHARE_FILE_RS *)buf;
    if(rs->result == 0)return;
    //先删除
    m_mainDialog->slot_deleteAllShareFileInfo();
    //刷新分享的列表
    slot_refreshMyShare();
}

void CKernel::slot_dealMyShareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_MY_SHARE_RS * rs = (STRU_MY_SHARE_RS *)buf;
    int n = rs->itemCount;
    for(int i = 0 ; i < n ; ++i){

        m_mainDialog->slot_insertShareFile(rs->items[i].name,FileInfo::getSize(rs->items[i].size)
                                           ,rs->items[i].time,QString::number(rs->items[i].shareLink));
        //QString size =  FileInfo::getSize(rs->items[i].size);
        //QString name = rs->items[i].name;
        //QString time = rs->items[i].time;
        //cout << name.toStdString() << size.toStdString() << time.toStdString() <<endl;
    }

}

void CKernel::slot_dealGetShareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    //解包
    STRU_GET_SHARE_RS * rs = (STRU_GET_SHARE_RS *)buf;
    if(rs->result == 0) return;

    if(QString::fromStdString(rs->dir) == m_curDir){
        slot_updateFileList();
    }
}

void CKernel::slot_dealContinueUploadRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug() << __func__;
    //拆包
    STRU_CONTINUE_UPLOAD_RS * rs = (STRU_CONTINUE_UPLOAD_RS *)buf;

    //发送文件块
    if( m_mapFileidToFileInfo.count(rs->fileid) == 0 ) return;
    FileInfo& info = m_mapFileidToFileInfo[ rs->fileid ]; //拿到 fileid 对应的文件信息

    //pos  跳转  pFile  pos设置  控件进度条位置
    fseek( info.pFile , rs->pos , SEEK_SET);
    info.pos = rs->pos;

    m_mainDialog->slot_updateUploadFileProgress( info.fileid , info.pos);

    STRU_FILE_CONTENT_RQ rq;
    //读文件
    int len = fread( rq.content , 1 , _DEF_BUFFER , info.pFile );
    rq.len = len;
    rq.fileid = rs->fileid;
    rq.userid = m_id;

    //发 文件内容
    SendData( (char * )&rq , sizeof(rq) );
}
//注册请求的结果
//#define tel_is_exist		(0)
//#define name_is_exist     (1)
//#define register_success	(2)
void CKernel::slot_dealRegisterRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<< __func__;
    //拆包
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS *)buf;
    //根据不同结果显示不同提示
    switch(rs->result){
    case tel_is_exist:
        QMessageBox::about(m_loginDialog,"注册提示：","手机号已注册，注册失败");
        break;
    case name_is_exist:
        QMessageBox::about(m_loginDialog,"注册提示：","昵称已存在，注册失败");
        break;
    case register_success:
        QMessageBox::about(m_loginDialog,"注册提示：","注册成功");
        break;

    }
}

//登录请求的结果
//#define user_not_exist		(0)
//#define password_error		(1)
//#define login_success		    (2)
void CKernel::slot_dealLoginRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<< __func__;
    //拆包
    STRU_LOGIN_RS * rs = (STRU_LOGIN_RS *)buf;
    //根据不同结果显示不同提示
    switch(rs->result){
    case user_not_exist:
        QMessageBox::about(m_loginDialog,"登录提示：","用户不存在，登录失败");
        break;
    case password_error:
        QMessageBox::about(m_loginDialog,"登录提示：","密码错误，登录失败");
        break;
    case login_success:
        //ui界面切换 前台
        m_loginDialog->hide();
        m_mainDialog->show();

        //后台
        m_name = rs->name;
        m_id = rs->userid;

        m_mainDialog->slot_setInfo(m_name);
        m_curDir = "/";

        //登录成功  追加请求
        //获取用户根目录'/'下的所有文件
//        STRU_FILE_LIST_RQ rq;
//        rq.userid = m_id;
//        strcpy(rq.dir,"/");
//        SendData((char*)&rq,sizeof(rq));
        slot_updateFileList();

        m_mainDialog->slot_deleteAllShareFileInfo();
        slot_refreshMyShare();

        initDatabase( m_id );

        break;

    }
}

void CKernel::slot_dealFileInfo(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_FILE_Info* info = (STRU_FILE_Info*)buf;
    //读信息
    qDebug() << "fileName:" << info->fileName
             << "fileid:" << info->fileid
             << "fileType:" << info->fileType
             << "size:" << info->size;

    //判断是不是当前的路径
    if(m_curDir == info->dir){
        FileInfo fileInfo;
        fileInfo.dir = QString::fromStdString(info->dir);
        fileInfo.fileid = info->fileid;
        fileInfo.name = QString::fromStdString(info->fileName);
        fileInfo.type = QString::fromStdString(info->fileType);

        fileInfo.md5 = QString::fromStdString(info->md5);
        fileInfo.size = info->size;
        fileInfo.time = QString::fromStdString(info->uploadTime);

        //添加到控件
        m_mainDialog->slot_insertFileInfo(fileInfo);
    }
}
void CKernel::SendData(char *buf, int nlen)
{
    m_tcpClient->SendData(0, buf, nlen);
}
void CKernel::initDatabase( int id)
{
    QString path = QCoreApplication::applicationDirPath() + "/database/";
    //先看路径再看文件
    QDir dr;
    if( !dr.exists(path) ){
        dr.mkdir(path);
    }
    path = path + QString("%1.db").arg(id);
    //在exe同级下面  查看有没有这个文件
    QFileInfo info(path);
    if( info.exists()){
        //存在  //有  加载数据库
        m_sql->ConnectSql( path );

        //查询 条数
//        QString sqlstr = QString("select count(*) from t_upload");
//        QStringList lst;
//        m_sql->SelectSql( sqlstr , 1 , lst );
//        if(lst.size() != 0){
//            qDebug() << "upload:" << lst.front() << "条";
//        }
//        lst.clear();
//        sqlstr = QString("select count(*) from t_download");

//        m_sql->SelectSql( sqlstr , 1 , lst );
//        if(lst.size() != 0){
//            qDebug() << "download:" << lst.front() << "条";
//        }

        //获取未完成的任务
        QList<FileInfo> uploadList;
        QList<FileInfo> downloadList;
        slot_getUploadTask(uploadList);
        slot_getDownloadTask(downloadList);

        //更新界面 --> 空间插入
        for(FileInfo& info : uploadList){

            QFileInfo fi(info.absolutePath);
            if( !fi.exists() ){
                continue; //上传文件无法找到 下一步
            }
            info.isPause = 1;//修改 初始状态 -> 开始
            m_mainDialog->slot_insertUploadFile( info );

            //发协议 向服务器 查看当前位置 todo

        }

        //更新界面 --> 空间插入
        for(FileInfo& info : downloadList){
            QFileInfo fi(info.absolutePath);
            if( !fi.exists() ){
                continue; //下载文件无法找到 下一步
            }
            info.isPause = 1;//修改 初始状态 -> 开始
            info.pos = fi.size();
            m_mainDialog->slot_insertDownloadFile( info );

            //下载  可以查看本地 看已完成大小
            m_mainDialog->slot_updateFileProgress( info.fileid, fi.size());

        }

    }else{
        //没有 创建
        QFile file(path);
        if(!file.open(QIODevice::WriteOnly)) return;
        file.close();

        m_sql->ConnectSql( path );
        //写表
        QString sqlstr = "create table t_upload ( f_id int , f_name varchar(260),f_dir varchar(260),f_absolutePath varchar(260),f_size int,f_md5 varchar(40),f_time varchar(40),f_type varchar(10));";
        m_sql->UpdateSql(sqlstr);

        sqlstr = "create table t_download ( f_id int , f_name varchar(260),f_dir varchar(260),f_absolutePath varchar(260),f_size int,f_md5 varchar(40),f_time varchar(40),f_type varchar(10));";
        m_sql->UpdateSql(sqlstr);
    }

}

//写上传任务
void CKernel::slot_writeUploadTask(FileInfo &info)
{
    QString sqlstr = QString("insert into t_upload values(%1,'%2','%3','%4',%5,'%6','%7','%8')").
            arg(info.fileid).arg(info.name).arg(info.dir).arg(info.absolutePath).arg(info.size).
            arg(info.md5).arg(info.time).arg(info.type);

    m_sql->UpdateSql( sqlstr );


}
//写下载任务
void CKernel::slot_writeDownloadTask(FileInfo &info)
{
    QString sqlstr = QString("insert into t_download values(%1,'%2','%3','%4',%5,'%6','%7','%8')").
            arg(info.fileid).arg(info.name).arg(info.dir).arg(info.absolutePath).arg(info.size).
            arg(info.md5).arg(info.time).arg(info.type);

    m_sql->UpdateSql( sqlstr );
}
//删除上传
void CKernel::slot_deleteUploadTask(FileInfo &info)
{
    QString sqlstr = QString("delete from t_upload where f_id = %1 and f_dir = '%2';").
            arg(info.fileid).arg(info.dir);
    m_sql->UpdateSql( sqlstr );
}
//删除下载
void CKernel::slot_deleteDownloadTask(FileInfo &info)
{
    QString sqlstr = QString("delete from t_download where f_id = %1 and f_dir = '%2' and f_absolutePath = '%3';").
            arg(info.fileid).arg(info.dir).arg(info.absolutePath);
    m_sql->UpdateSql( sqlstr );
}
//加载上传任务
void CKernel::slot_getUploadTask(QList<FileInfo> &infoList)
{
    QString sqlstr = "select * from t_upload;";
    QStringList lst;
    m_sql->SelectSql(sqlstr , 8 , lst);
    while (lst.size()) {

        FileInfo info;
        info.fileid = lst.front().toInt(); lst.pop_front();
        info.name = lst.front();lst.pop_front();
        info.dir = lst.front();lst.pop_front();
        info.absolutePath = lst.front();lst.pop_front();
        info.size = lst.front().toInt(); lst.pop_front();
        info.md5 = lst.front();lst.pop_front();
        info.time = lst.front();lst.pop_front();
        info.type = lst.front();lst.pop_front();
        infoList.push_back(info);
    }
}
//加载下载任务
void CKernel::slot_getDownloadTask(QList<FileInfo> &infoList)
{
    QString sqlstr = "select * from t_download;";
    QStringList lst;
    m_sql->SelectSql(sqlstr , 8 , lst);
    while (lst.size()) {

        FileInfo info;
        info.fileid = lst.front().toInt(); lst.pop_front();
        info.name = lst.front();lst.pop_front();
        info.dir = lst.front();lst.pop_front();
        info.absolutePath = lst.front();lst.pop_front();
        info.size = lst.front().toInt(); lst.pop_front();
        info.md5 = lst.front();lst.pop_front();
        info.time = lst.front();lst.pop_front();
        info.type = lst.front();lst.pop_front();
        infoList.push_back(info);
    }
}


void CKernel::slot_clientReadyData(unsigned int lSendIP, char *buf, int nlen)
{

//    std::string text = buf;
//    QMessageBox::about(nullptr , "客户端接收",
//                       QString("来自服务器:%1").arg(QString::fromStdString(text)));
    //协议映射关系
    //协议头 -- 判断包是干嘛用的

//    char* tmp = buf;
//    int type = *(int*)tmp;//按照int 取数据
//    //判断拿出来的协议头是否合法， type需要在协议范围
//    if(type >= DEF_PACK_BASE && type < DEF_PACK_COUNT + DEF_PACK_BASE)
//    {
//        switch(type)
//        {
//            case DEF_LOGIN_RS:
//            //调用处理登录回复

//            break;
//        }
//    }
    //map 协议头 -> 函数指针
        char* tmp = buf;
        int type = *(int*)tmp;//按照int 取数据
        //判断拿出来的协议头是否合法， type需要在协议范围
        if(type >= _DEF_PROTOCOL_BASE && type < _DEF_PROTOCOL_COUNT + _DEF_PROTOCOL_BASE)
        {
            PFUN pf = NetMap(type);
            if(pf)
                (this->*pf)(lSendIP,buf,nlen);//pf为类成员函数指针，要拿对象调用
        }
        delete[] buf;
}


