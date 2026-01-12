#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
bool initDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("StudentGrades.db");
    if (!db.open()) return false;

    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS grades ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "name TEXT, "
                  "class_name TEXT, "
                  "subject TEXT, "
                  "score REAL, "
                  "exam_date DATE)";
    return query.exec(sql);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    if (!initDatabase()) return -1;
    QFile styleFile(":/style.qss");
    if(styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
    LoginDialog login; // 现在这里不会报错了
    if (login.exec() == QDialog::Accepted) {
        MainWindow w;
        w.setWindowTitle("学生成绩管理系统");
        w.show();
        return a.exec();
    }
    return 0;
}
