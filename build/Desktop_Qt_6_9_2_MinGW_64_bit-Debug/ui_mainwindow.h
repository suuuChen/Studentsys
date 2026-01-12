/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCharts/QChartView>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *label_3;
    QLineEdit *leSearchName;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *comboFilter;
    QLabel *label_2;
    QComboBox *comboClass;
    QChartView *chartViewTrend;
    QTableView *tableView;
    QPushButton *btnBatchInput;
    QPushButton *btnDelete;
    QPushButton *btnAnalysis;
    QPushButton *btnExport;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(753, 617);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");

        verticalLayout->addWidget(label_3);

        leSearchName = new QLineEdit(centralwidget);
        leSearchName->setObjectName("leSearchName");

        verticalLayout->addWidget(leSearchName);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");

        verticalLayout->addLayout(horizontalLayout);

        label = new QLabel(centralwidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        comboFilter = new QComboBox(centralwidget);
        comboFilter->addItem(QString());
        comboFilter->addItem(QString());
        comboFilter->addItem(QString());
        comboFilter->addItem(QString());
        comboFilter->setObjectName("comboFilter");

        verticalLayout->addWidget(comboFilter);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        comboClass = new QComboBox(centralwidget);
        comboClass->setObjectName("comboClass");

        verticalLayout->addWidget(comboClass);

        chartViewTrend = new QChartView(centralwidget);
        chartViewTrend->setObjectName("chartViewTrend");
        chartViewTrend->setMinimumSize(QSize(500, 0));

        verticalLayout->addWidget(chartViewTrend);

        tableView = new QTableView(centralwidget);
        tableView->setObjectName("tableView");
        tableView->setMinimumSize(QSize(0, 200));

        verticalLayout->addWidget(tableView);

        btnBatchInput = new QPushButton(centralwidget);
        btnBatchInput->setObjectName("btnBatchInput");
        btnBatchInput->setMinimumSize(QSize(0, 40));

        verticalLayout->addWidget(btnBatchInput);

        btnDelete = new QPushButton(centralwidget);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setMinimumSize(QSize(0, 40));

        verticalLayout->addWidget(btnDelete);

        btnAnalysis = new QPushButton(centralwidget);
        btnAnalysis->setObjectName("btnAnalysis");
        btnAnalysis->setMinimumSize(QSize(0, 40));

        verticalLayout->addWidget(btnAnalysis);

        btnExport = new QPushButton(centralwidget);
        btnExport->setObjectName("btnExport");
        btnExport->setMinimumSize(QSize(0, 40));

        verticalLayout->addWidget(btnExport);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 753, 18));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "\346\220\234\347\264\242", nullptr));
        leSearchName->setText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "\347\255\233\351\200\211\347\247\221\347\233\256", nullptr));
        comboFilter->setItemText(0, QCoreApplication::translate("MainWindow", "\345\205\250\351\203\250", nullptr));
        comboFilter->setItemText(1, QCoreApplication::translate("MainWindow", "\346\225\260\345\255\246", nullptr));
        comboFilter->setItemText(2, QCoreApplication::translate("MainWindow", "\350\213\261\350\257\255", nullptr));
        comboFilter->setItemText(3, QCoreApplication::translate("MainWindow", "\350\257\255\346\226\207", nullptr));

        label_2->setText(QCoreApplication::translate("MainWindow", "\347\255\233\351\200\211\347\217\255\347\272\247", nullptr));
        btnBatchInput->setText(QCoreApplication::translate("MainWindow", "\345\242\236\345\212\240\344\270\200\350\241\214", nullptr));
        btnDelete->setText(QCoreApplication::translate("MainWindow", "\345\210\240\351\231\244\351\200\211\344\270\255\350\241\214", nullptr));
        btnAnalysis->setText(QCoreApplication::translate("MainWindow", "\346\237\245\347\234\213\345\217\212\346\240\274\347\216\207\345\210\206\346\236\220", nullptr));
        btnExport->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272\347\273\237\350\256\241\346\212\245\350\241\250", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
