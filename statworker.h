#ifndef STATWORKER_H
#define STATWORKER_H

#include <QObject>
#include <QSqlTableModel>
#include "gradestatistics.h"




class StatWorker : public QObject
{
    Q_OBJECT
public:
    explicit StatWorker(QObject *parent = nullptr);


public slots:
    // 后台执行的统计任务
    void doWork(QSqlTableModel *model,const QString &filterType, const QString &filterValue);

signals:
    // 任务完成后发送这个信号，把统计结果传回给界面
    void resultReady(const GradeStatistics &stats);
    void errorOccurred(const QString &error);


};

#endif // STATWORKER_H
