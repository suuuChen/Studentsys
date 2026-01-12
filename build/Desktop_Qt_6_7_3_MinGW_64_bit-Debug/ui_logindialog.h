/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QLineEdit *lePass;
    QLineEdit *leUser;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnLogin;
    QPushButton *btnCancel;
    QLabel *label_2;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName("LoginDialog");
        LoginDialog->resize(370, 312);
        layoutWidget = new QWidget(LoginDialog);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(70, 70, 251, 161));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        lePass = new QLineEdit(layoutWidget);
        lePass->setObjectName("lePass");
        lePass->setEchoMode(QLineEdit::EchoMode::Password);

        gridLayout->addWidget(lePass, 4, 0, 1, 1);

        leUser = new QLineEdit(layoutWidget);
        leUser->setObjectName("leUser");

        gridLayout->addWidget(leUser, 1, 0, 1, 1);

        label = new QLabel(layoutWidget);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        btnLogin = new QPushButton(layoutWidget);
        btnLogin->setObjectName("btnLogin");
        btnLogin->setMinimumSize(QSize(100, 40));

        horizontalLayout->addWidget(btnLogin);

        btnCancel = new QPushButton(layoutWidget);
        btnCancel->setObjectName("btnCancel");
        btnCancel->setMinimumSize(QSize(100, 40));

        horizontalLayout->addWidget(btnCancel);


        gridLayout->addLayout(horizontalLayout, 6, 0, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 2, 0, 1, 1);


        retranslateUi(LoginDialog);

        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("LoginDialog", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        btnLogin->setText(QCoreApplication::translate("LoginDialog", "\347\231\273\345\275\225", nullptr));
        btnCancel->setText(QCoreApplication::translate("LoginDialog", "\345\217\226\346\266\210", nullptr));
        label_2->setText(QCoreApplication::translate("LoginDialog", "\345\257\206\347\240\201\357\274\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
