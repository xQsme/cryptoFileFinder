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
    if(fileToCheck.size() < 32)
    {
        return;
    }
    QString command = fileCommand(file);
    if(true || command.contains("enc'd") || command.contains("encrypted") || (command.contains("data") && !command.contains("image") && !command.contains("archive")))
    {
        total=0;
        QHash<char, long> data;
        while(!fileToCheck.atEnd())
        {
            QByteArray read = fileToCheck.read(1);
            data[read[0]]++;
            total++;
        }
        double entropy = fileEntropy(data);
        if(entropy > 7)
        {
            double chi2 = calculateChi2(data);
            double limit= 22017.84 + (374.6088 - 22017.84)/(1.0 + pow((1.0*fileToCheck.size()/2269952000), 0.8129303));
            if(chi2 < limit)
            {
                nGrams(&fileToCheck);
                /*double piError = approximatePi(&fileToCheck);
                if(piError < 0.2 && piError > 0.1)*/
                count++;
                qDebug() << command;
                *stream << command.split(": ")[1] << endl;
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
    for(int i=2; i <= 16; i*=2)
    {
        file->reset();
        total=0;
        QHash<char, long> data;
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
                file->seek(file->pos()-1);
            }
            data[static_cast<char>(current)]++;
            total++;
        }
        *stream << fileEntropy(data) << ";";
        *stream << calculateChi2(data) << ";";
    }
}

void Thread::nGramSequence(QFile* file)
{
    for(int i=2; i <= 16; i*=2)
    {
        file->reset();
        total=0;
        QHash<QByteArray, long> data;
        while(!file->atEnd())
        {
            data[file->read(i)]++;
            total++;
            if(!file->atEnd())
            {
                file->seek(file->pos()-1);
            }
        }
        *stream << nGramEntropy(data) << ";";
        *stream << nGramChi2(data) << ";";
    }
}

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
    qDebug() << entropy;
    return entropy;
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
