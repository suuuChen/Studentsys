#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("系统登录");
    ui->lePass->setEchoMode(QLineEdit::Password);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::on_btnLogin_clicked() {
    if(ui->leUser->text() == "admin" && ui->lePass->text() == "123456") {
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！");
    }
}

void LoginDialog::on_btnCancel_clicked() {
    reject();
}
