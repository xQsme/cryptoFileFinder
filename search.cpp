#include "search.h"

Search::Search(QObject *parent) : QObject(parent)
{

}

void Search::setStuff(QString dir, QString file, int fast, int bytes)
{
    this->dir=dir;
    this->file=file;
    this->fast=fast;
    this->bytes=bytes;
}

void Search::search()
{
    QFile output(file);
    output.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&output);
    totalThreads = QThread::idealThreadCount();
    QList<Thread*> threads;
    for(int i = 0; i < totalThreads; i++)
    {
        threads.append(new Thread(i, totalThreads, dir, &stream, fast, bytes));
        connect(threads[i], SIGNAL(ended(int)), this, SLOT(ended(int)));
    }
    qDebug() << "Searching encrypted files with " + QString::number(totalThreads) + " thread(s).";
    endedThreads=0;
    count=0;
    for(int i = 0; i < totalThreads; i++)
    {
        threads[i]->run();
    }
}

void Search::ended(int found)
{
    endedThreads++;
    count+=found;
    if(endedThreads==totalThreads)
    {
        qDebug() << "Found " + QString::number(count) + " encryted files.";
    }
}
