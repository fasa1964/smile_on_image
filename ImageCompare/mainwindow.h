#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QMap>
#include <QTime>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVariantMap>

#include <comparethread.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void closeButtonClicked();
    void startButtonClicked();
    void pathButtonClicked();
    void deleteButtonClicked();
    void infoButtonClicked();

    void formatBoxTextChanged(const QString &);

    // CheckBox
    void allFormatsStateChanged(int);
    void subdirectoriesStateChanged(int);

    // QLineEdit
    void pathEditTextChanged(const QString &text);


    // QListWidget
    void itemClicked(QListWidgetItem *item);

    // Compare Approximation
    void approximationSliderChanged(int value);



    // CompareThread signals
    void currentCompare(const QString &s, const QString &d);
    void threadError(const QString &error);
    void identcalImageOcuured(const QString &s, const QString &d);
    void processFinished();

    void process0Step(int value);
    void process1Step(int value);
    void process2Step(int value);
    void process3Step(int value);

private:
    Ui::MainWindow *ui;

    QTime start;
    int processStepCounter;
    void mainProcess();

    QStringList identicalImageList;
    QMap<QString, QPair<QString,QString>> identicalImageMap;
    QMap<int, QString> fileMap;
    void setupFileMap();

    bool pathExist(const QString &path);
    void setTextColor(QColor color, QWidget *widget);

    QStringList toStringList(QList<QByteArray> list);

    void setupThreading();
    void setupConnecting(CompareThread *thread);

    QMap <int, CompareThread *> threadMap;
    QMap <int, QPair<int, int>> threadRange;

    void readSettings();
    void writeSettings();
};

#endif // MAINWINDOW_H
