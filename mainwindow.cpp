#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statworker.h"
#include <QSqlTableModel>
#include <QInputDialog>
#include <QSqlQuery>
#include <QDate>
#include <QMessageBox>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QHeaderView>
#include <QSqlRecord>
#include <QtCharts>
#include <QBarCategoryAxis>
#include <QCategoryAxis>
#include <QMargins>
#include <QFileDialog>  // 用来弹窗选保存位置
#include <QFile>        // 用来创建文件
#include <QTextStream>  // 用来往文件里写字
#include <QSqlError>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->chartViewTrend->setMinimumHeight(200);
    // 初始化数据库
    model = new QSqlTableModel(this);
    model->setTable("grades");
    model->setSort(5, Qt::DescendingOrder); // 按考试日期降序
    model->select();
    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0, true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 初始化下拉框
    ui->comboClass->clear();
    ui->comboClass->addItems({"全部", "一班", "二班"});

    ui->comboFilter->clear();
    ui->comboFilter->addItems({"全部", "数学", "语文", "英语"});

    // 应用初始筛选（显示所有数据）
    applyFilters();

    // 初始化统计工作线程
    statWorker = new StatWorker();
    workerThread = new QThread();
    statWorker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, statWorker, &QObject::deleteLater);
    connect(this, &MainWindow::startStatistics, statWorker, &StatWorker::doWork);
    connect(statWorker, &StatWorker::resultReady, this, &MainWindow::onStatsReady);

    workerThread->start();

    ui->statusbar->showMessage("系统就绪");
}

MainWindow::~MainWindow() {
    // 停止工作线程
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }

    // 清理图表
    if (ui->chartViewTrend && ui->chartViewTrend->chart()) {
        // 注意：不要直接删除，让Qt管理
        ui->chartViewTrend->setChart(nullptr);
    }
    delete ui;
}

// 趋势图逻辑 (注意：没有任何 QtCharts:: 前缀，直接用类名)
void MainWindow::updateTrendChart(QString name) {
    // 安全检查
    if (!ui->chartViewTrend) {
        qDebug() << "chartViewTrend is null";
        return;
    }

    // 保存旧的图表对象指针
    QChart *oldChart = ui->chartViewTrend->chart();

    // 创建新的图表
    QChart *chart = new QChart();
    chart->setTitle(name + " 成绩趋势图");

    if (name.isEmpty()) {
        // 显示班级平均趋势或分数分布
        QString className = ui->comboClass->currentText();
        if (className != "全部") {
            // 这里应该调用 updateClassTrendChart，但为了避免递归，我们直接处理
            // 先删除旧的，再调用
            if (oldChart) {
                delete oldChart;
            }
            updateClassTrendChart(className);
            return;
        } else {
            if (oldChart) {
                delete oldChart;
            }
            updateScoreDistributionChart();
            return;
        }
    }

    QString currentSubject = ui->comboFilter->currentText();
    QSqlQuery query;

    // 查询个人成绩趋势
    QString sql = "SELECT subject, score, exam_date FROM grades WHERE name = ?";
    if (currentSubject != "全部") {
        sql += " AND subject = '" + currentSubject + "'";
    }
    sql += " ORDER BY exam_date ASC";

    query.prepare(sql);
    query.addBindValue(name);

    QMap<QString, QLineSeries*> subjectSeries;
    QDate firstDate;

    if (query.exec()) {
        while (query.next()) {
            QString subject = query.value(0).toString();
            double score = query.value(1).toDouble();
            QDate date = QDate::fromString(query.value(2).toString(), "yyyy-MM-dd");

            if (!firstDate.isValid() || date < firstDate) {
                firstDate = date;
            }

            if (!subjectSeries.contains(subject)) {
                QLineSeries *series = new QLineSeries();
                series->setName(subject);
                subjectSeries[subject] = series;
            }

            int dayOffset = firstDate.daysTo(date);
            subjectSeries[subject]->append(dayOffset, score);
        }
    }

    if (subjectSeries.isEmpty()) {
        chart->setTitle(name + " - 暂无数据");
    } else {
        for (auto series : subjectSeries) {
            chart->addSeries(series);
        }
        chart->legend()->setVisible(true);
        chart->createDefaultAxes();

        // 设置Y轴范围
        QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
        if (axisY) {
            axisY->setRange(0, 100);
            axisY->setTitleText("分数");
        }
    }

    // 设置新的图表
    ui->chartViewTrend->setChart(chart);
    ui->chartViewTrend->setRenderHint(QPainter::Antialiasing);

    // 删除旧的图表（必须在设置新图表之后）
    if (oldChart) {
        // 注意：不要删除 oldChart，因为 setChart 会自动管理
        // 但我们可以显式清理
        oldChart->deleteLater();
    }

    qDebug() << "趋势图更新完成";
}

void MainWindow::on_btnBatchInput_clicked() {
    bool ok;
    // 提示用户可以输入日期了
    QString text = QInputDialog::getMultiLineText(this, "批量录入",
                                                  "格式：姓名,班级,科目,分数,日期(可选)\n例如：张三,一班,数学,90,2025-12-01", "", &ok);

    if (!ok || text.isEmpty()) return;

    QSqlDatabase::database().transaction();
    int successCount = 0;
    int failCount = 0;
    QStringList errorMessages;

    // 有效数据定义
    QStringList validSubjects = {"数学", "语文", "英语"};
    QStringList validClasses = {"一班", "二班"};

    QStringList lines = text.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QStringList data = line.split(",");
        for (int i = 0; i < data.size(); ++i) {
            data[i] = data[i].trimmed();
        }

        QString errorMsg;
        if (!validateBatchInputLine(data, errorMsg)) {
            errorMessages << QString("行 '%1': %2").arg(line).arg(errorMsg);
            failCount++;
            continue;
        }

        // 执行插入
        QSqlQuery query;
        query.prepare("INSERT INTO grades (name, class_name, subject, score, exam_date) VALUES (?,?,?,?,?)");
        query.addBindValue(data[0]);
        query.addBindValue(data[1]);
        query.addBindValue(data[2]);
        query.addBindValue(data[3].toDouble());

        // 处理日期
        if (data.size() >= 5 && !data[4].isEmpty()) {
            QDate date = QDate::fromString(data[4], "yyyy-MM-dd");
            if (!date.isValid()) {
                date = QDate::fromString(data[4], "yyyy/MM/dd");
            }
            query.addBindValue(date);
        } else {
            query.addBindValue(QDate::currentDate());
        }

        if (!query.exec()) {
            failCount++;
            QString error = query.lastError().text();  // 先获取错误文本
            errorMessages << QString("数据库错误: %1").arg(error);
        }
    }

    QSqlDatabase::database().commit();

    // 刷新数据（保持当前筛选条件）
    applyFilters();  // 改为调用 applyFilters() 而不是 model->select()

    // 显示结果
    QString message = QString("导入完成：成功 %1 条，失败 %2 条").arg(successCount).arg(failCount);
    if (!errorMessages.isEmpty()) {
        message += "\n错误详情：" + errorMessages.join("\n");
        QMessageBox::warning(this, "导入警告", message);
    } else {
        ui->statusbar->showMessage(message, 5000);
    }

    // 更新图表
    if (!ui->leSearchName->text().isEmpty()) {
        updateTrendChart(ui->leSearchName->text());
    }
}

void MainWindow::on_btnDelete_clicked() {
    int row = ui->tableView->currentIndex().row();
    if (row >= 0) {
        model->removeRow(row);
        applyFilters();  // 改为调用 applyFilters()
    }
}

void MainWindow::on_comboClass_currentIndexChanged(int index) {
    Q_UNUSED(index);
    applyFilters();
}

void MainWindow::on_comboFilter_currentIndexChanged(int index) {
    Q_UNUSED(index);
    applyFilters();
}

void MainWindow::on_btnAnalysis_clicked() {
    // 触发统计计算
    QString filterType = "all";
    QString filterValue = "";

    QString currentSubject = ui->comboFilter->currentText();
    QString currentClass = ui->comboClass->currentText();

    if (currentSubject != "全部") {
        filterType = "subject";
        filterValue = currentSubject;
    } else if (currentClass != "全部") {
        filterType = "class";
        filterValue = currentClass;
    }

    emit startStatistics(model, filterType, filterValue);
}
void MainWindow::handleResults(int p, int f) { Q_UNUSED(p); Q_UNUSED(f); }
void MainWindow::on_btnExport_clicked() {
    // 先计算统计
    GradeStatistics stats = calculateStatistics("");

    // 生成报表内容
    QString htmlReport = generateStatisticsReport(stats);

    // 选择导出格式
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this, "导出统计报表",
        QString("成绩统计_%1_%2").arg(QDate::currentDate().toString("yyyyMMdd"))
            .arg(QTime::currentTime().toString("hhmmss")),
        "HTML文件 (*.html);;PDF文件 (*.pdf);;CSV文件 (*.csv)",
        &selectedFilter
        );

    if (fileName.isEmpty()) return;

    bool success = false;
    if (selectedFilter.contains("HTML")) {
        success = exportToHTML(fileName, htmlReport);
    } else if (selectedFilter.contains("PDF")) {
        success = exportToPDF(fileName, htmlReport);
    } else {
        // CSV导出（使用原有逻辑，但可以增强）
        success = exportToCSV(fileName, stats);
    }

    if (success) {
        QMessageBox::information(this, "成功", "报表导出成功！");
    } else {
        QMessageBox::warning(this, "错误", "报表导出失败！");
    }
}

QString MainWindow::generateStatisticsReport(const GradeStatistics &stats) {
    QString report = QString(R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <title>成绩统计报告</title>
            <style>
                body { font-family: 'Microsoft YaHei', Arial, sans-serif; margin: 40px; }
                h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }
                h2 { color: #34495e; margin-top: 30px; }
                table { border-collapse: collapse; width: 100%; margin: 20px 0; }
                th, td { border: 1px solid #ddd; padding: 12px; text-align: center; }
                th { background-color: #3498db; color: white; }
                tr:nth-child(even) { background-color: #f8f9fa; }
                .stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; }
                .stat-card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
                .stat-value { font-size: 24px; font-weight: bold; color: #2c3e50; }
                .stat-label { color: #7f8c8d; margin-top: 5px; }
                .good { color: #27ae60; }
                .warning { color: #f39c12; }
                .danger { color: #e74c3c; }
            </style>
        </head>
        <body>
            <h1>📊 成绩统计报告</h1>
            <p><strong>生成时间：</strong>%1</p>
            <p><strong>筛选条件：</strong>班级：%2 | 科目：%3</p>

            <h2>📈 核心统计指标</h2>
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-value">%4</div>
                    <div class="stat-label">参考人数</div>
                </div>
                <div class="stat-card">
                    <div class="stat-value">%5</div>
                    <div class="stat-label">平均分</div>
                </div>
                <div class="stat-card">
                    <div class="stat-value %6">%7%</div>
                    <div class="stat-label">及格率</div>
                </div>
                <div class="stat-card">
                    <div class="stat-value %8">%9%</div>
                    <div class="stat-label">优秀率</div>
                </div>
            </div>
    )").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                         .arg(ui->comboClass->currentText())
                         .arg(ui->comboFilter->currentText())
                         .arg(stats.totalCount)
                         .arg(QString::number(stats.average, 'f', 2))
                         .arg(stats.passRate >= 80 ? "good" : stats.passRate >= 60 ? "warning" : "danger")
                         .arg(QString::number(stats.passRate, 'f', 1))
                         .arg(stats.excellentRate >= 20 ? "good" : stats.excellentRate >= 10 ? "warning" : "danger")
                         .arg(QString::number(stats.excellentRate, 'f', 1));

    // 添加详细统计表格
    report += QString(R"(
        <h2>📋 详细统计数据</h2>
        <table>
            <tr><th>项目</th><th>数值</th></tr>
            <tr><td>最高分</td><td>%1</td></tr>
            <tr><td>最低分</td><td>%2</td></tr>
            <tr><td>及格人数</td><td>%3</td></tr>
            <tr><td>不及格人数</td><td>%4</td></tr>
            <tr><td>优秀人数(≥90)</td><td>%5</td></tr>
            <tr><td>良好人数(≥80)</td><td>%6</td></tr>
            <tr><td>中等人数(≥70)</td><td>%7</td></tr>
        </table>
    )").arg(stats.maxScore)
                  .arg(stats.minScore)
                  .arg(stats.passCount)
                  .arg(stats.failCount)
                  .arg(stats.excellentCount)
                  .arg(stats.goodCount)
                  .arg(stats.mediumCount);

    // 添加分数段分布
    report += "<h2>📊 分数段分布</h2><table><tr><th>分数段</th><th>人数</th><th>占比</th></tr>";
    for (auto it = stats.scoreDistribution.begin(); it != stats.scoreDistribution.end(); ++it) {
        QString rangeName;
        switch (it.key()) {
        case 90: rangeName = "优秀(90-100)"; break;
        case 80: rangeName = "良好(80-89)"; break;
        case 70: rangeName = "中等(70-79)"; break;
        case 60: rangeName = "及格(60-69)"; break;
        case 0: rangeName = "不及格(0-59)"; break;
        }
        if (it.value() > 0) {
            double percentage = stats.totalCount > 0 ?
                                    it.value() * 100.0 / stats.totalCount : 0;
            report += QString("<tr><td>%1</td><td>%2</td><td>%3%</td></tr>")
                          .arg(rangeName)
                          .arg(it.value())
                          .arg(QString::number(percentage, 'f', 1));
        }
    }
    report += "</table>";

    // 添加各科平均分
    if (!stats.subjectAverages.isEmpty()) {
        report += "<h2>📚 各科平均分</h2><table><tr><th>科目</th><th>平均分</th></tr>";
        for (auto it = stats.subjectAverages.begin(); it != stats.subjectAverages.end(); ++it) {
            report += QString("<tr><td>%1</td><td>%2</td></tr>")
            .arg(it.key())
                .arg(QString::number(it.value(), 'f', 2));
        }
        report += "</table>";
    }

    report += R"(
            <footer style="margin-top: 50px; padding-top: 20px; border-top: 1px solid #ddd; color: #7f8c8d; text-align: center;">
                <p>Generated by 学生成绩管理系统 | © 2025</p>
            </footer>
        </body>
        </html>
    )";

    return report;
}

bool MainWindow::exportToHTML(const QString &filename, const QString &htmlContent) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    // Qt 6 兼容性处理
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    // 写入 BOM 以确保 Excel 正确识别中文
    out << "\xEF\xBB\xBF";

    out << htmlContent;
    file.close();

    return true;
}

bool MainWindow::exportToPDF(const QString &filename, const QString &htmlContent) {
    QMessageBox::information(this, "提示",
                             "PDF导出需要Qt的PDF模块支持。\n"
                             "建议先导出为HTML，然后用浏览器打印为PDF。\n\n"
                             "或者安装Qt PDF模块：\n"
                             "在.pro文件中添加：QT += pdf\n"
                             "然后使用QPdfWriter类。");

    QString htmlFilename = filename;
    if (htmlFilename.endsWith(".pdf", Qt::CaseInsensitive)) {
        htmlFilename = htmlFilename.left(htmlFilename.length() - 4) + ".html";
    } else {
        htmlFilename += ".html";
    }

    return exportToHTML(htmlFilename, htmlContent);
}

bool MainWindow::exportToCSV(const QString &filename, const GradeStatistics &stats) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
// Qt 6 中 setCodec 已被移除，使用 setEncoding
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF";  // UTF-8 BOM
#else
    out.setCodec("UTF-8");
    out << "\xEF\xBB\xBF";  // UTF-8 BOM
#endif

    // 写入表头
    out << "统计项目,数值\n";
    out << "参考人数," << stats.totalCount << "\n";
    out << "平均分," << QString::number(stats.average, 'f', 2) << "\n";
    out << "最高分," << stats.maxScore << "\n";
    out << "最低分," << stats.minScore << "\n";
    out << "及格人数," << stats.passCount << "\n";
    out << "不及格人数," << stats.failCount << "\n";
    out << "及格率," << QString::number(stats.passRate, 'f', 1) << "%\n";
    out << "优秀率," << QString::number(stats.excellentRate, 'f', 1) << "%\n";

    // 写入分数段分布
    out << "\n分数段分布\n";
    out << "分数段,人数,占比\n";
    for (auto it = stats.scoreDistribution.begin(); it != stats.scoreDistribution.end(); ++it) {
        QString rangeName;
        switch (it.key()) {
        case 90: rangeName = "优秀(90-100)"; break;
        case 80: rangeName = "良好(80-89)"; break;
        case 70: rangeName = "中等(70-79)"; break;
        case 60: rangeName = "及格(60-69)"; break;
        case 0: rangeName = "不及格(0-59)"; break;
        }
        if (it.value() > 0) {
            double percentage = stats.totalCount > 0 ?
                                    it.value() * 100.0 / stats.totalCount : 0;
            out << rangeName << "," << it.value() << ","
                << QString::number(percentage, 'f', 1) << "%\n";
        }
    }

    file.close();
    return true;
}


bool MainWindow::validateBatchInputLine(const QStringList &data, QString &errorMsg) {
    if (data.size() < 4) {
        errorMsg = "数据不完整，至少需要4个字段";
        return false;
    }
    // 验证姓名不能为空
    if (data[0].trimmed().isEmpty()) {
        errorMsg = "姓名不能为空";
        return false;
    }

    // 验证班级
    static QStringList validClasses = {"一班", "二班"};
    if (!validClasses.contains(data[1])) {
        errorMsg = QString("无效班级：%1").arg(data[1]);
        return false;
    }

    // 验证科目
    static QStringList validSubjects = {"数学", "语文", "英语"};
    if (!validSubjects.contains(data[2])) {
        errorMsg = QString("无效科目：%1").arg(data[2]);
        return false;
    }

    // 验证分数
    bool ok;
    double score = data[3].toDouble(&ok);
    if (!ok) {
        errorMsg = "分数必须是数字";
        return false;
    }
    if (score < 0 || score > 100) {
        errorMsg = "分数必须在0-100之间";
        return false;
    }

    // 验证日期（如果有）
    if (data.size() >= 5 && !data[4].isEmpty()) {
        QDate date = QDate::fromString(data[4], "yyyy-MM-dd");
        if (!date.isValid()) {
            date = QDate::fromString(data[4], "yyyy/MM/dd");
        }
        if (!date.isValid()) {
            errorMsg = "日期格式错误，请使用yyyy-MM-dd或yyyy/MM/dd";
            return false;
        }
        if (date > QDate::currentDate()) {
            errorMsg = "考试日期不能晚于今天";
            return false;
        }
    }

    return true;
}

void MainWindow::updateClassTrendChart(const QString &className) {
    if (!ui->chartViewTrend) return;

    // 创建新的图表
    QChart *chart = new QChart();

    // 查询班级各科目平均分趋势
    QSqlQuery query;
    query.prepare(R"(
        SELECT subject, AVG(score) as avg_score, exam_date
        FROM grades
        WHERE class_name = ?
        GROUP BY subject, exam_date
        ORDER BY exam_date ASC
    )");
    query.addBindValue(className);

    QMap<QString, QLineSeries*> subjectSeries;
    QDate firstDate;

    if (query.exec()) {
        while (query.next()) {
            QString subject = query.value(0).toString();
            double avgScore = query.value(1).toDouble();
            QDate date = QDate::fromString(query.value(2).toString(), "yyyy-MM-dd");

            if (!firstDate.isValid() || date < firstDate) {
                firstDate = date;
            }

            if (!subjectSeries.contains(subject)) {
                QLineSeries *series = new QLineSeries();
                series->setName(subject);
                subjectSeries[subject] = series;
            }

            int dayOffset = firstDate.daysTo(date);
            subjectSeries[subject]->append(dayOffset, avgScore);
        }
    }

    if (subjectSeries.isEmpty()) {
        chart->setTitle(className + " - 暂无数据");
    } else {
        for (auto series : subjectSeries) {
            chart->addSeries(series);
        }

        chart->setTitle(className + " 各科目平均分趋势");
        chart->legend()->setVisible(true);
        chart->createDefaultAxes();
    }

    // 设置图表
    ui->chartViewTrend->setChart(chart);
    ui->chartViewTrend->setRenderHint(QPainter::Antialiasing);

    qDebug() << "班级趋势图更新完成";
}

void MainWindow::updateScoreDistributionChart() {
    if (!ui->chartViewTrend) return;

    // 创建新的图表
    QChart *chart = new QChart();

    // 计算分数段分布
    GradeStatistics stats = calculateStatistics();

    QPieSeries *series = new QPieSeries();

    // 添加各分数段数据
    for (auto it = stats.scoreDistribution.begin(); it != stats.scoreDistribution.end(); ++it) {
        QString label;
        switch (it.key()) {
        case 90: label = "优秀(90-100)"; break;
        case 80: label = "良好(80-89)"; break;
        case 70: label = "中等(70-79)"; break;
        case 60: label = "及格(60-69)"; break;
        case 0: label = "不及格(0-59)"; break;
        }
        if (it.value() > 0) {
            series->append(label, it.value());
        }
    }

    if (series->count() == 0) {
        delete series;
        chart->setTitle("暂无数据");
    } else {
        // 设置切片标签
        for (QPieSlice *slice : series->slices()) {
            slice->setLabelVisible();
            slice->setLabel(QString("%1\n%2人 (%3%)")
                                .arg(slice->label())
                                .arg(slice->value())
                                .arg(QString::number(slice->percentage() * 100, 'f', 1)));
        }

        chart->addSeries(series);
        chart->setTitle("分数段分布图");
        chart->legend()->setVisible(true);
    }

    // 设置图表
    ui->chartViewTrend->setChart(chart);
    ui->chartViewTrend->setRenderHint(QPainter::Antialiasing);

    qDebug() << "分数分布图更新完成";
}
GradeStatistics MainWindow::calculateStatistics(const QString &filter) {
    GradeStatistics stats;

    // 根据filter构建查询条件
    QString whereClause;
    QString currentFilter = filter;

    // 如果filter为空，根据当前UI状态判断
    if (currentFilter.isEmpty()) {
        QString currentSubject = ui->comboFilter->currentText();
        QString currentClass = ui->comboClass->currentText();

        if (currentSubject != "全部") {
            whereClause = QString(" WHERE subject = '%1'").arg(currentSubject);
        } else if (currentClass != "全部") {
            whereClause = QString(" WHERE class_name = '%1'").arg(currentClass);
        }
        // 如果都是"全部"，whereClause保持为空，表示统计全部数据
    } else {
        // 使用传入的filter
        whereClause = filter;
    }

    QSqlQuery query;

    // 1. 基本统计信息
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

    // 2. 各科平均分（如果不是按科目筛选）
    QString currentSubject = ui->comboFilter->currentText();
    if (currentSubject == "全部") {
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

    // 3. 分数段分布
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

    return stats;
}

GradeStatistics MainWindow::calculateSingleSubjectStats(const QString &subject) {
    // 直接调用通用函数，添加科目筛选条件
    QString whereClause = QString(" WHERE subject = '%1'").arg(subject);
    return calculateStatistics(whereClause);
}
GradeStatistics MainWindow::calculateClassStats(const QString &className) {
    // 直接调用通用函数，添加班级筛选条件
    QString whereClause = QString(" WHERE class_name = '%1'").arg(className);
    return calculateStatistics(whereClause);
}
GradeStatistics MainWindow::calculateAllSubjectsStats() {
    // 直接调用通用函数，不添加筛选条件
    return calculateStatistics("");
}
void MainWindow::showStatisticsInStatusBar(const GradeStatistics &stats) {
    QString message = QString("统计结果：共%1人 | 平均分：%2 | 及格率：%3% | 优秀率：%4%")
                          .arg(stats.totalCount)
                          .arg(QString::number(stats.average, 'f', 1))
                          .arg(QString::number(stats.passRate, 'f', 1))
                          .arg(QString::number(stats.excellentRate, 'f', 1));

    ui->statusbar->showMessage(message, 10000); // 显示10秒
}

void MainWindow::applyFilters() {
    QStringList conditions;

    // 1. 姓名筛选（模糊搜索）
    QString nameFilter = ui->leSearchName->text().trimmed();
    if (!nameFilter.isEmpty()) {
        conditions.append(QString("name LIKE '%%1%'").arg(nameFilter));
    }

    // 2. 班级筛选
    QString classFilter = ui->comboClass->currentText();
    if (classFilter != "全部") {
        conditions.append(QString("class_name = '%1'").arg(classFilter));
    }

    // 3. 科目筛选
    QString subjectFilter = ui->comboFilter->currentText();
    if (subjectFilter != "全部") {
        conditions.append(QString("subject = '%1'").arg(subjectFilter));
    }

    // 组合所有条件
    QString filterStr;
    if (!conditions.isEmpty()) {
        filterStr = conditions.join(" AND ");
    }

    // 应用筛选
    model->setFilter(filterStr);
    model->select();

    // 更新状态栏显示筛选结果数量
    int rowCount = model->rowCount();
    ui->statusbar->showMessage(QString("找到 %1 条记录").arg(rowCount), 3000);

    // 更新图表（添加安全检查）
    if (ui->chartViewTrend) {
        QString nameFilter = ui->leSearchName->text().trimmed();
        if (!nameFilter.isEmpty()) {
            updateTrendChart(nameFilter);
        } else {
            QString classFilter = ui->comboClass->currentText();
            if (classFilter != "全部") {
                updateClassTrendChart(classFilter);
            } else {
                updateScoreDistributionChart();
            }
        }
    }
}

void MainWindow::onStatsReady(const GradeStatistics &stats) {
    // 显示统计结果
    QString message = QString("统计完成：共%1条记录，平均分：%2，及格率：%3%，优秀率：%4%")
                          .arg(stats.totalCount)
                          .arg(QString::number(stats.average, 'f', 1))
                          .arg(QString::number(stats.passRate, 'f', 1))
                          .arg(QString::number(stats.excellentRate, 'f', 1));

    ui->statusbar->showMessage(message, 5000);


}

void MainWindow::on_leSearchName_textChanged(const QString &arg1) {
    Q_UNUSED(arg1);

    // 调用统一的筛选函数
    applyFilters();

    // 如果需要显示个人总分，可以保留这个逻辑
    QString name = ui->leSearchName->text().trimmed();
    if (!name.isEmpty()) {
        QSqlQuery query;
        query.prepare("SELECT SUM(score) FROM grades WHERE name = ?");
        query.addBindValue(name);
        if (query.exec() && query.next()) {
            double totalScore = query.value(0).toDouble();
            if (totalScore > 0) {
                ui->statusbar->showMessage(QString("%1 总分: %2").arg(name).arg(totalScore), 5000);
            }
        }
    }
}
