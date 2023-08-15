#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QRegExp>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("注册&登录");
    //Qt::FramelessWindowHint无边框
    //this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                         Qt::WindowCloseButtonHint);
    //一直保持在注册的分页里
    ui->tw_login_register->setCurrentIndex(1);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
//注册的提交
void LoginDialog::on_pb_commit_register_clicked()
{
    //从控件中获取文本
    QString tel = ui->le_tel_register->text();
    QString passwd = ui->le_password_register->text();
    QString confirm = ui->le_confim_register->text();
    QString name = ui->le_name_register->text();

    //验证
    //首先所有都要非空
    if(tel.isEmpty() || passwd.isEmpty() ||
            confirm.isEmpty() || name.isEmpty()){
        QMessageBox::about(this,"注册提示","输入不可为空");
        return;
    }
    //验证手机号--正则表达式
    QRegExp exp(QString("^1[356789][0-9]\{9\}$"));
    bool res = exp.exactMatch(tel);
    if(!res){
        QMessageBox::about(this,"注册提示:","电话号格式不对");
        return;
    }
    //验证密码
    if(passwd.length() > 20){
        QMessageBox::about(this,"注册提示:","输入密码过长");
        return;
    }
    //验证密码和确认
    if(passwd != confirm){
        QMessageBox::about(this,"注册提示:","两次密码输入不一致");
        return;
    }
    //验证昵称--长度、敏感词
    if(name.length() > 10){
        QMessageBox::about(this,"注册提示:","输入昵称过长");
        return;
    }
    //发送信号(发送给核心类)
    Q_EMIT SIG_registerCommit(tel,passwd,name);
    //
}
//登陆的提交
void LoginDialog::on_pb_commit_clicked()
{
    //从控件中获取文本
    QString tel = ui->le_tel->text();
    QString passwd = ui->le_password->text();

    //验证
    //首先所有都要非空
    if(tel.isEmpty() || passwd.isEmpty()){
        QMessageBox::about(this,"注册提示","输入不可为空");
        return;
    }
    //验证手机号--正则表达式
    QRegExp exp(QString("^1[356789][0-9]\{9\}$"));
    bool res = exp.exactMatch(tel);
    if(!res){
        QMessageBox::about(this,"注册提示:","电话号格式不对");
        return;
    }
    //验证密码
    if(passwd.length() > 20){
        QMessageBox::about(this,"注册提示:","输入密码过长");
        return;
    }

    //发送信号(发送给核心类)
    Q_EMIT SIG_loginCommit(tel, passwd);
}

//注册清空
void LoginDialog::on_pb_clear_register_clicked()
{
    ui->le_tel_register->setText("");
    ui->le_password_register->setText("");
    ui->le_confim_register->setText("");
    ui->le_name_register->setText("");
}

//登录清空
void LoginDialog::on_pb_clear_clicked()
{
    ui->le_tel->setText("");
    ui->le_password->setText("");
}

void LoginDialog::closeEvent(QCloseEvent *event)
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



