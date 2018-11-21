#include "thread.h"

Thread::Thread(int thread, int totalThreads, QString dir, QTextStream* stream, int fast, int bytes)
{
    this->thread=thread;
    this->totalThreads=totalThreads;
    this->dir=dir;
    this->stream=stream;
    this->fast=fast;
    this->bytes=bytes;
    currentFile=0;
    count=0;
}

void Thread::run()
{
    search(dir, stream);
    emit ended(count);
}

void Thread::search(QString dir, QTextStream* stream)
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
                    analyzeFile(file.absoluteFilePath(), stream);
                }
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d, stream);
    }
}

void Thread::analyzeFile(QString file, QTextStream* stream)
{
    QFile fileToCheck(file);
    fileToCheck.open(QIODevice::ReadOnly);
    if(fileEntropy(&fileToCheck))
    {
        count++;
        if(!fast)
        {
            *stream << file + ": encrypted with " + fileLength() + " block size." << endl;
            qDebug() << file + ": encrypted with " + fileLength() + " block size.";
        }
        else
        {
            *stream << file << endl;
            qDebug() << file;
        }
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
    QHash<char, int> data;
    while(!file->atEnd())
    {
        QByteArray read = file->read(1);
        data[read[0]]++;
        total++;
        if(fast && total >= bytes)
        {
            break;
        }
    }
    QHashIterator<char, int> i(data);
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
    if(entropy<6)
    {
        return 0;
    }
    return compressionVsEncryption(data, max);
}

int Thread::compressionVsEncryption(QHash<char, int> data, int max){
    float avg = total/data.keys().length();
    QHashIterator<char, int> i(data);
    float chi2=0;
    float nSuccess=0;
    while (i.hasNext())
    {
        i.next();
        chi2+=(i.value()-avg)*(i.value()-avg)/avg;
        if(i.value()+avg <= max){
            nSuccess++;
        }
    }
    float piDiff=abs((nSuccess/data.keys().length()-M_PI_4)/M_PI_4);
    if(chi2 < 450)
    {
        qDebug() << "Pi error: " << piDiff;
        return 1;
    }
    return 0;
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
