#ifndef COMMON_H
#define COMMON_H

#include <QString>

struct FileInfo
{
    FileInfo():fileid(0),size(0),pFile(nullptr),pos(0),isPause(0){

    }
    int fileid;
    QString name;
    QString dir;
    QString time;
    int size;
    QString md5;
    QString type;
    //std::string absolutePath;
    QString absolutePath;
    int pos;//上传或下载到什么位置

    int isPause;//暂停 0 1

    //文件指针
    FILE* pFile;


    static QString getSize(int size) //...kb  ...Mb  3030byte
    //3030 -> 3.03KB
    {
        QString res;
       //看 能被1024除几次  0次 kb  1次 KB  2次  MB  再往上太大了
       int tmp = size;
       int count = 0;
       while(tmp != 0){
           tmp /= 1024;
           if(tmp != 0) count++;
       }
       switch(count){
       case 0:
           res = QString("0.%1KB").arg((int)(size/1024.0*1000) , 3 ,10 ,QChar('0'));
           break;
       case 1:
           res = QString("%1.%2KB").arg(size/1024).
                   arg((int)(size%1024/1024.0*1000) , 3 , 10 , QChar('0'));
           break;
       case 2:
           res = QString("%1.%2MB").arg(size/1024/1024 ).
                   arg((int)(size/1024%1024/1024.0*1000) , 3 , 10 , QChar('0'));
           break;
       default:
           res = QString("%1.%2MB").arg(size/1024/1024 ).
                   arg((int)(size/1024%1024/1024.0*1000) , 3 , 10 , QChar('0'));
           break;
       }
       return res;
    }
};




#endif // COMMON_H
