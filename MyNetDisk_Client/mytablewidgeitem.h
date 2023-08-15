#ifndef MYTABLEWIDGEITEM_H
#define MYTABLEWIDGEITEM_H

#include <QTableWidgetItem>
#include "common.h"

class MyTableWidgeItem : public QTableWidgetItem
{
//    Q_OBJECT
public:
    explicit MyTableWidgeItem();

    void setInfo(FileInfo& info);
public:
    FileInfo m_info;


};

#endif // MYTABLEWIDGEITEM_H
