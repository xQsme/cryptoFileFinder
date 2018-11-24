#ifndef THREAD_H
#define THREAD_H
#include <QtCore>

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread(int thread, int totalThreads, QString dir, QTextStream* stream);
    void run();
private:
    int thread;
    int totalThreads;
    QString dir;
    QTextStream* stream;
    long total;
    int count;
    int currentFile;
    void search(QString dir);
    void analyzeFile(QString file);
    float fileEntropy(QHash<char, long> data);
    float calculateChi2(QHash<char, long> data);
    float approximatePi(QFile* file);
    QString fileCommand(QString file);
signals:
    void ended(int thread);
};

#endif // THREAD_H
