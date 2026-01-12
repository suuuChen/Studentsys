#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
// 【关键】必须在这里包含图表头文件，否则编译器不认识 QtCharts
#include <QtCharts>
#include "statworker.h"
#include "gradestatistics.h"  // 包含结构体定义

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    // 添加这个信号，用于启动统计工作
    void startStatistics(QSqlTableModel* model, const QString& filterType, const QString& filterValue);

private slots:
    void on_btnBatchInput_clicked();
    void on_btnDelete_clicked();
    void on_leSearchName_textChanged(const QString &arg1);
    void on_comboClass_currentIndexChanged(int index);
    void on_comboFilter_currentIndexChanged(int index);
    void on_btnAnalysis_clicked();
    void on_btnExport_clicked();
    void handleResults(int pass, int fail);




    // 新增：统计结果显示
    void onStatsReady(const GradeStatistics &stats);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *model;
    StatWorker *statWorker;
    QThread *workerThread;

    // 核心功能方法
    void updateTrendChart(QString name);
    void updateClassTrendChart(const QString &className);
    void updateScoreDistributionChart();

    // 统计方法
    GradeStatistics calculateStatistics(const QString &filter = "");
    GradeStatistics calculateSingleSubjectStats(const QString &subject);
    GradeStatistics calculateClassStats(const QString &className);
    GradeStatistics calculateAllSubjectsStats();

    // 报表生成
    QString generateStatisticsReport(const GradeStatistics &stats);
    bool exportToPDF(const QString &filename, const QString &htmlContent);
    bool exportToHTML(const QString &filename, const QString &htmlContent);
    bool exportToCSV(const QString &filename, const GradeStatistics &stats);

    // 批量录入验证
    bool validateBatchInputLine(const QStringList &data, QString &errorMsg);

    // 工具方法
    void showStatisticsInStatusBar(const GradeStatistics &stats);

    void applyFilters();
};
#endif // MAINWINDOW_H
