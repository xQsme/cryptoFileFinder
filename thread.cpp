#include "thread.h"

Thread::Thread(int thread, int totalThreads, QString dir, QTextStream* stream)
{
    this->thread=thread;
    this->totalThreads=totalThreads;
    this->dir=dir;
    this->stream=stream;
    currentFile=0;
    count=0;
}

void Thread::run()
{
    search(dir);
    emit ended(count);
}

void Thread::search(QString dir)
{
    QDir root(dir);
    QList<QString> dirs;
    foreach(QFileInfo file, root.entryInfoList())
    {
        if(!root.absolutePath().contains(file.absoluteFilePath()))
        {
            if(file.isDir() && file.absoluteFilePath().left(2)!="/.")
            {
                dirs.append(file.absoluteFilePath());
            }
            else
            {
                currentFile++;
                if(currentFile % totalThreads == thread)
                {
                    analyzeFile(file.absoluteFilePath());
                }
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d);
    }
}

void Thread::analyzeFile(QString file)
{
    QFile fileToCheck(file);
    fileToCheck.open(QIODevice::ReadOnly);
    if(fileEntropy(&fileToCheck))
    {
        count++;
        *stream << file.split(".").last() << endl;
        qDebug() << file.split(".").last();
    }
    fileToCheck.close();
}

int Thread::fileEntropy(QFile* file)
{
    if(file->size() < 32)
    {
        return 0;
    }
    total=0;
    QHash<char, long> data;
    while(!file->atEnd())
    {
        QByteArray read = file->read(1);
        data[read[0]]++;
        total++;
    }
    QHashIterator<char, long> i(data);
    float entropy=0;
    int max=0;
    while (i.hasNext())
    {
        i.next();
        float current = 1.0*i.value()/total;
        entropy -= current*log2(current);
        if(i.value() > max){
            max = i.value();
        }
    }
    if(entropy < 7.5)
    {
        return 0;
    }

    if(compressionVsEncryption(data))
    {
        qDebug() << "Size:" << file->size();
        *stream << file->size() << ";";
        qDebug() << "Entropy:" << entropy;
        *stream << entropy << ";";
        approximatePi(file);
        return 1;
    }
    return 0;
}

int Thread::compressionVsEncryption(QHash<char, long> data){
    float avg = total/data.keys().length();
    QHashIterator<char, long> i(data);
    float chi2=0;
    while (i.hasNext())
    {
        i.next();
        chi2+=(i.value()-avg)*(i.value()-avg)/avg;
    }
    if(chi2 < 300)
    {
        qDebug() << "Chi^2:" << chi2;
        *stream << chi2 << ";";
        return 1;
    }
    return 0;
}

int Thread::approximatePi(QFile* file)
{
    file->reset();
    int value;
    int iterations=3;
    int nSuccess=0;
    int count=0;
    while(!file->atEnd())
    {
        value=0;
        for(int i = 0; i < iterations; i++)
        {
            if(file->atEnd())
            {
                break;
            }
            value += file->read(1)[0];
        }
        count++;
        if(value <= 128 && value >= -128){
            nSuccess++;
        }
    }
    float piDiff=abs((1.0*nSuccess/count-M_PI_4)/M_PI_4);
    qDebug() << "Pi error: " << piDiff;
    *stream << piDiff << ";";
    return 1;
}

QString Thread::fileLength()
{
   for(int i = 32; i <= 512; i+=32)
   {
       if(!total%i)
       {
           return QString::number(i) + " bit multiple";
       }
   }
   return "unknown";
}
