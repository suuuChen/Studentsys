#ifndef GRADESTATISTICS_H
#define GRADESTATISTICS_H

#include <QMap>
#include <QString>

struct GradeStatistics {
    int totalCount = 0;
    double average = 0.0;
    double maxScore = 0.0;
    double minScore = 100.0;
    int passCount = 0;          // 及格人数(>=60)
    int failCount = 0;          // 不及格人数(<60)
    int excellentCount = 0;     // 优秀人数(>=90)
    int goodCount = 0;          // 良好人数(>=80)
    int mediumCount = 0;        // 中等人数(>=70)
    double passRate = 0.0;      // 及格率
    double excellentRate = 0.0; // 优秀率
    QMap<QString, double> subjectAverages; // 各科平均分
    QMap<int, int> scoreDistribution; // 分数段分布
};

#endif // GRADESTATISTICS_H
