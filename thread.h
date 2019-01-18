#ifndef THREAD_H
#define THREAD_H
#include <QtCore>

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread(int thread, int totalThreads, QString dir, int testing, int nonRecursive);
    void run();
private:
    int thread;
    int totalThreads;
    QString dir;
    QTextStream* stream;
    long total;
    int count;
    int currentFile;
    int testing;
    int nonRecursive;
    void search(QString dir);
    void analyzeFile(QString file);
    double fileEntropy(QHash<char, long> data);
    double calculateChi2(QHash<char, long> data);
    double nGramSequence(QFile* file);
    double nGramChi2(QHash<QByteArray, long> data);
    QString fileCommand(QString file);
    /*double nGramEntropy(QHash<QByteArray, long> data);
    double approximatePi(QFile* file);
    void nGrams(QFile* file);
    double sumEntropy(QHash<int, long> data);
    double sumChi2(QHash<int, long> data);*/
signals:
    void content(QString content);
    void ended(int thread);
};

#endif // THREAD_H
