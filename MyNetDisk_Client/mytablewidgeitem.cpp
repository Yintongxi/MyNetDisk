#include "mytablewidgeitem.h"

MyTableWidgeItem::MyTableWidgeItem()
{

}

void MyTableWidgeItem::setInfo(FileInfo &info)
{
    //后台的处理
    m_info = info;
    //前台的显示
    this->setText(info.name);
    if(info.type == "dir")
        this->setIcon(QIcon(":/images/file.png"));
    else
        this->setIcon(QIcon(":/images/folder.png"));
    this->setCheckState(Qt::Unchecked);


}
