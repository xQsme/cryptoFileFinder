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
    int fileEntropy(QFile* file);
    QString fileLength();
    int compressionVsEncryption(QHash<char, long> data);
    int approximatePi(QFile* file);
signals:
    void ended(int thread);
};

#endif // THREAD_H
