#ifndef COMPARETHREAD_H
#define COMPARETHREAD_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QImage>

class CompareThread : public QThread
{
    Q_OBJECT

public:
    CompareThread();

    void compare();
    void stop();

    QMap<int, QString> imageMap() const;
    void setImageMap(const QMap<int, QString> &imageMap);

    // test
    void setRange(int start, int end);

    bool getFast() const;
    void setFast(bool fast);

    int getApproximationValue() const;
    void setApproximationValue(int approximationValue);

public slots:
    void incomingCF(const QString &srcname, const QString &destname);


signals:
    void error(const QString &err);
    void compareFile(const QString &srcname, const QString &destname);
    void identicalOccured(const QString &srcname, const QString &destname);
    void processGrowth(int value);


protected:
    void run();

private:
    volatile bool stopped;
    QMap<int, QString> m_imageMap;

    QImage getImage(int index);
    bool compareImage(const QImage &s, const QImage &d);

    int endpos;
    int startpos;

    // Test Approximation
    int imagepixels;
    int equalpix;
    int notequalpix;
    bool m_fast;
    int m_approximationValue;

    qreal percentOf(int ipix);
};

#endif // COMPARETHREAD_H
