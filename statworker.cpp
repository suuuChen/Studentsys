#include "statworker.h"
#include <QSqlQuery>
#include <QDebug>
#include<QThread>
#include<QSqlError>

StatWorker::StatWorker(QObject *parent) : QObject(parent) {}



void StatWorker::doWork(QSqlTableModel *model,const QString &filterType, const QString &filterValue)
{

    GradeStatistics stats;

    // 构建查询条件
    QString whereClause;
    if (filterType == "subject" && filterValue != "全部") {
        whereClause = QString(" WHERE subject = '%1'").arg(filterValue);
    } else if (filterType == "class" && filterValue != "全部") {
        whereClause = QString(" WHERE class_name = '%1'").arg(filterValue);
    }

    QSqlQuery query;

    // 基本统计

    // 基本统计
    QString sql = QString(R"(
        SELECT
            COUNT(*) as total,
            AVG(score) as avg_score,
            MAX(score) as max_score,
            MIN(score) as min_score,
            COUNT(CASE WHEN score >= 60 THEN 1 END) as pass_count,
            COUNT(CASE WHEN score >= 90 THEN 1 END) as excellent_count,
            COUNT(CASE WHEN score >= 80 AND score < 90 THEN 1 END) as good_count,
            COUNT(CASE WHEN score >= 70 AND score < 80 THEN 1 END) as medium_count
        FROM grades %1
    )").arg(whereClause);

    if (query.exec(sql) && query.next()) {
        stats.totalCount = query.value(0).toInt();
        stats.average = query.value(1).toDouble();
        stats.maxScore = query.value(2).toDouble();
        stats.minScore = query.value(3).toDouble();
        stats.passCount = query.value(4).toInt();
        stats.excellentCount = query.value(5).toInt();
        stats.goodCount = query.value(6).toInt();
        stats.mediumCount = query.value(7).toInt();
        stats.failCount = stats.totalCount - stats.passCount;

        if (stats.totalCount > 0) {
            stats.passRate = stats.passCount * 100.0 / stats.totalCount;
            stats.excellentRate = stats.excellentCount * 100.0 / stats.totalCount;
        }
    }

    // 各科平均分统计
    if (filterType != "subject") {
        sql = QString(R"(
            SELECT subject, AVG(score) as avg_score
            FROM grades %1
            GROUP BY subject
        )").arg(whereClause);

        if (query.exec(sql)) {
            while (query.next()) {
                QString subject = query.value(0).toString();
                double avg = query.value(1).toDouble();
                stats.subjectAverages[subject] = avg;
            }
        }
    }

    // 分数段分布统计
    sql = QString(R"(
        SELECT
            CASE
                WHEN score >= 90 THEN 90
                WHEN score >= 80 THEN 80
                WHEN score >= 70 THEN 70
                WHEN score >= 60 THEN 60
                ELSE 0
            END as score_range,
            COUNT(*) as count
        FROM grades %1
        GROUP BY score_range
        ORDER BY score_range
    )").arg(whereClause);

    if (query.exec(sql)) {
        while (query.next()) {
            int range = query.value(0).toInt();
            int count = query.value(1).toInt();
            stats.scoreDistribution[range] = count;
        }
    }

    emit resultReady(stats);
}
