#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QCloseEvent>
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_registerCommit(QString tel,QString passwd,QString name);
    void SIG_loginCommit(QString tel,QString passwd);
    void SIG_close();
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_pb_commit_register_clicked();

    void on_pb_clear_register_clicked();

    void on_pb_clear_clicked();

    void on_pb_commit_clicked();

private:
    Ui::LoginDialog *ui;
    //关闭事件  同意会发送关闭信号
    void closeEvent(QCloseEvent *event);
};

#endif // LOGINDIALOG_H
