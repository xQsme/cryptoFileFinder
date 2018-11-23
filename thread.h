#ifndef THREAD_H
#define THREAD_H
#include <QtCore>

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread(int thread, int totalThreads, QString dir, QTextStream* stream, int fast, int bytes);
    void run();
private:
    int thread;
    int totalThreads;
    QString dir;
    QTextStream* stream;
    int fast;
    int bytes;
    int total;
    int count;
    int currentFile;
    void search(QString dir, QTextStream* stream);
    void analyzeFile(QString file, QTextStream* stream);
    int fileEntropy(QFile* file);
    QString fileLength();
    int compressionVsEncryption(QHash<char, int> data, int max, QFile* file);
    int approximatePi(QFile* file);
signals:
    void ended(int thread);
};

#endif // THREAD_H
