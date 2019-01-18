#include "thread.h"

Thread::Thread(int thread, int totalThreads, QString dir, int testing,int nonRecursive)
{
    this->thread=thread;
    this->totalThreads=totalThreads;
    this->dir=dir;
    this->testing=testing;
    this->nonRecursive=nonRecursive;
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
            if(file.isDir() && file.absoluteFilePath().left(2)!="/." && !nonRecursive)
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
    if(fileToCheck.size() < 32)
    {
        return;
    }
    fileToCheck.open(QIODevice::ReadOnly);
    total=0;
    QHash<char, long> data;
    while(!fileToCheck.atEnd())
    {
        QByteArray read = fileToCheck.read(1);
        data[read[0]]++;
        total++;
    }
    double entropy = fileEntropy(data);
    if(testing || entropy > 7.3)
    {
        double chi2 = calculateChi2(data);
        double limit= 22017.84 + (374.6088 - 22017.84)/(1.0 + pow((1.0*fileToCheck.size()/2269952000), 0.8129303));
        if(testing || chi2 < limit)
        {
            double nGramChi2 = nGramSequence(&fileToCheck);
            double limit2;
            if(fileToCheck.size() < 1000000)
            {
                limit2 = 1.335831 + 0.0004732094*fileToCheck.size() + 0.00000003104824*fileToCheck.size() * fileToCheck.size();
            }
            else
            {
                limit2 = 18715610 + (-20651.77 - 18715610)/(1.0 + pow((1.0*fileToCheck.size()/25331810), 1.824073));
            }
            if(testing || nGramChi2 < limit2)
            {
                count++;
                QString command = fileCommand(file);
                qDebug() << command;
                if(testing)
                {
                    emit content(QString::number(entropy) + ";" + QString::number(chi2) + ";" +
                                 QString::number(limit) + ";" + QString::number(nGramChi2) + ";" +
                                 QString::number(limit2) + ";" + QString::number(fileToCheck.size()) + ";" +
                                 file.split(".").last() + ";" + command.split(": ")[1]);
                }
                else
                {
                    emit content(command);
                }

            }
        }
    }
    fileToCheck.close();
}

double Thread::fileEntropy(QHash<char, long> data)
{
    QHashIterator<char, long> i(data);
    double entropy=0;
    while (i.hasNext())
    {
        i.next();
        double current = 1.0*i.value()/total;
        entropy -= current*log2(current);
    }
    return entropy;
}

double Thread::calculateChi2(QHash<char, long> data){
    double avg = total/data.size();
    QHashIterator<char, long> i(data);
    double chi2=0;
    while (i.hasNext())
    {
        i.next();
        chi2+=(i.value()-avg)*(i.value()-avg)/avg;
    }
    return chi2;
}

double Thread::nGramSequence(QFile* file)
{
    file->reset();
    total=0;
    QHash<QByteArray, long> data;
    while(!file->atEnd())
    {
        data[file->read(3)]++;
        total++;
        if(!file->atEnd())
        {
            file->seek(file->pos()-2);
        }
    }
    return nGramChi2(data);
}

double Thread::nGramChi2(QHash<QByteArray, long> data){
    double avg = total/data.size();
    QHashIterator<QByteArray, long> i(data);
    double chi2=0;
    while (i.hasNext())
    {
        i.next();
        chi2+=(i.value()-avg)*(i.value()-avg)/avg;
    }
    return chi2;
}

QString Thread::fileCommand(QString file)
{
    QProcess process;
    QStringList args;
    args << file;
    process.start("file", args);
    process.waitForFinished();
    QString output(process.readAllStandardOutput());
    return output;
}

/* UNUSED FUNCTIONS

double Thread::nGramEntropy(QHash<QByteArray, long> data)
{
    QHashIterator<QByteArray, long> i(data);
    double entropy=0;
    while (i.hasNext())
    {
        i.next();
        double current = 1.0*i.value()/total;
        entropy -= current*log2(current);
    }
    return entropy;
}

double Thread::approximatePi(QFile* file)
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
    return abs((1.0*nSuccess/count-M_PI_4)/M_PI_4);
}

void Thread::nGrams(QFile* file)
{
    for(int i=2; i <= 4; i++)
    {
        file->reset();
        total=0;
        QHash<int, long> data;
        while(!file->atEnd())
        {
            int current = 0;
            for(int k = 0; k < i; k++)
            {
                if(!file->atEnd())
                {
                    current += file->read(1)[0];
                }
            }
            if(!file->atEnd())
            {
                file->seek(file->pos()-i+1);
            }
            data[current]++;
            total++;
        }
        *stream << sumEntropy(data) << ";";
        *stream << sumChi2(data) << ";";
    }
}

double Thread::sumEntropy(QHash<int, long> data)
{
    QHashIterator<int, long> i(data);
    double entropy=0;
    while (i.hasNext())
    {
        i.next();
        double current = 1.0*i.value()/total;
        entropy -= current*log2(current);
    }
    return entropy;
}

double Thread::sumChi2(QHash<int, long> data){
    double avg = total/data.size();
    QHashIterator<int, long> i(data);
    double chi2=0;
    while (i.hasNext())
    {
        i.next();
        chi2+=(i.value()-avg)*(i.value()-avg)/avg;
    }
    return chi2;
}
*/
