#include "search.h"

Search::Search(QObject *parent) : QObject(parent)
{

}

void Search::setStuff(QString dir, QString file, int fast, int bytes, int toUnmount)
{
    this->dir=dir;
    this->file=file;
    this->fast=fast;
    this->bytes=bytes;
    this->toUnmount=toUnmount;
}

void Search::search()
{
    output = new QFile(file);
    output->open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(output);
    totalThreads = QThread::idealThreadCount();
    QList<Thread*> threads;
    for(int i = 0; i < totalThreads; i++)
    {
        threads.append(new Thread(i, totalThreads, dir, &stream, fast, bytes));
        connect(threads[i], SIGNAL(ended(int)), this, SLOT(ended(int)));
    }
    qDebug() << "Searching encrypted files with " + QString::number(totalThreads) + " thread(s).";
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
        output->close();
        if(toUnmount)
        {
            qDebug() << "Unmounting partitions...";
            if (QProcess::execute(QString("/bin/sh") + " ./umount.sh") < 0){
                qDebug() << "Failed to run unmount script.";
            }
            qDebug() << "Done unmounting.";
        }
        qDebug() << "Found " + QString::number(count) + " encryted files.";
    }
}
