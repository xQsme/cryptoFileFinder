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
    double fileEntropy(QHash<char, long> data);
    double calculateChi2(QHash<char, long> data);
    double approximatePi(QFile* file);
    void nGrams(QFile* file);
    void nGramSequence(QFile* file);
    double nGramEntropy(QHash<QByteArray, long> data);
    double nGramChi2(QHash<QByteArray, long> data);
    QString fileCommand(QString file);
signals:
    void ended(int thread);
};

#endif // THREAD_H
