#include "search.h"

Search::Search(QObject *parent) : QObject(parent)
{

}

void Search::setStuff(QString dir, QString file, int toUnmount, QCoreApplication* app)
{
    this->dir=dir;
    this->file=file;
    this->toUnmount=toUnmount;
    this->app=app;
}

int Search::search()
{
    output = new QFile(file);
    output->open(QIODevice::WriteOnly | QIODevice::Text);
    stream = new QTextStream(output);
    totalThreads = QThread::idealThreadCount();
    QList<Thread*> threads;
    for(int i = 0; i < totalThreads; i++)
    {
        threads.append(new Thread(i, totalThreads, dir));
        connect(threads[i], SIGNAL(ended(int)), this, SLOT(ended(int)));
        connect(threads[i], SIGNAL(content(QString)), this, SLOT(content(QString)));
    }
    qDebug() << "Searching encrypted files with " + QString::number(totalThreads) + " thread(s).";
    *stream << "entropy;chi^2;limit;3-gram Chi^2;3-gram limit;Size;Termination;File Command" << endl;
    for(int i = 0; i < totalThreads; i++)
    {
        threads[i]->start();
    }
    return app->exec();
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
        app->exit();
    }
}

void Search::content(QString content)
{
    *stream << content;
}
