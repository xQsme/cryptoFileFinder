#include <QCoreApplication>
#include <QtDebug>
#include <QCommandLineParser>
#include <QDir>
#include <QProcess>
#include <QHash>
#include <tgmath.h>

int fast;
int bytes;
int count;
int total;

void mountPartitions();
void unmount();
void search(QString dir, QTextStream* stream);
void analyzeFile(QString file, QTextStream* stream);
bool fileEntropy(QFile* file);
QString fileLength();
void help();

int main(int argc, char *argv[])
{
    int match=0;
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption helpOption(QStringList() << "h" << "help",
                QCoreApplication::translate("main", "Usage Help"));
    parser.addOption(helpOption);
    QCommandLineOption mountOption(QStringList() << "m" << "mount",
                QCoreApplication::translate("main", "Mount all partitions"));
    parser.addOption(mountOption);
    QCommandLineOption searchOption(QStringList() << "s" << "search",
                QCoreApplication::translate("main", "Search for encrypted files"));
    parser.addOption(searchOption);
    QCommandLineOption fastOption(QStringList() << "f" << "fast",
                QCoreApplication::translate("main", "Read a maximum number of bytes from each file"));
    QCommandLineOption targetBytesOption(QStringList() << "b" << "bytes",
                QCoreApplication::translate("main", "Number of bytes to read from each file."),
                QCoreApplication::translate("main", "bytes"));
    parser.addOption(targetBytesOption);
    parser.addOption(fastOption);
    QCommandLineOption targetOutputOption(QStringList() << "o" << "output",
                QCoreApplication::translate("main", "Output file."),
                QCoreApplication::translate("main", "file"));
    parser.addOption(targetOutputOption);
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "dir" << "directory",
                QCoreApplication::translate("main", "Specify <directory>."),
                QCoreApplication::translate("main", "directory"));
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    QStringList args = parser.optionNames();
    if(args.contains("h") || args.contains("help"))
    {
        help();
        return 0;
    }
    int mounted = 0;
    if(args.contains("m")  || args.contains("mount"))
    {
        match=1;
        qDebug() << "Mounting partitions...";
        mountPartitions();
        mounted = 1;
    }
    QString dir;
    if(args.contains("d")  || args.contains("dir") || args.contains("directory"))
    {
        dir=parser.value(targetDirectoryOption);
        QDir root(dir);
        if(!root.exists()){
            qDebug() << "Invalid directory: " << dir;
            return -1;
        }
    }else{
        dir = "/root/dev";
    }
    QString file;
    if(args.contains("o") || args.contains("output"))
    {
        file=parser.value(targetOutputOption);
    }
    else
    {
        file="output.txt";
    }
    if(args.contains("f") || args.contains("fast"))
    {
        if(args.contains("b") || args.contains("bytes"))
        {
            QRegExp re("\\d*");
            if(re.exactMatch(parser.value(targetBytesOption)))
            {
                bytes=parser.value(targetBytesOption).toInt();
                if(bytes < 2048){
                    bytes=2048;
                    qDebug() << "Byte value too low, defaulting to 2048.";
                }
            }
            else
            {
                qDebug() << parser.value(fastOption) << " is not a valid number";
                return -1;
            }
        }
        else
        {
            bytes=2048;
        }
        fast=1;
    }
    else
    {
        fast=0;
    }
    if(args.contains("s")  || args.contains("search"))
    {
        match=1;
        qDebug() << "Searching encrypted files...";
        QFile output(file);
        count=0;
        output.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream stream(&output);
        search(dir, &stream);
        qDebug() << "Found " << count << " encrypted files.";
    }
    if(mounted == 1)
    {
        qDebug() << "Unmounting partitions...";
        unmount();
    }
    if(match == 0){
        help();
    }
    return 0;//a.exec();
}

void mountPartitions()
{
    if (QProcess::execute("chmod +x script.sh") >= 0)
    {
        if (QProcess::execute(QString("/bin/sh") + " ./script.sh") < 0)
        {
            qDebug() << "Failed to run mount script.";
        }
        else
        {
            qDebug() << "Mounted at \"~/dev\".";
        }
    }
}

void unmount()
{
    if (QProcess::execute(QString("/bin/sh") + " ./umount.sh") < 0){
        qDebug() << "Failed to run unmount script.";
    }else{
        qDebug() << "Unmounted all partitions.";
    }
}

void search(QString dir, QTextStream* stream)
{
    QDir root(dir);
    QList<QString> dirs;
    foreach(QFileInfo file, root.entryInfoList())
    {
        if(!root.absolutePath().contains(file.absoluteFilePath()))
        {
            if(file.isDir())
            {
                dirs.append(file.absoluteFilePath());
            }
            else
            {
                analyzeFile(file.absoluteFilePath(), stream);
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d, stream);
    }
}

void analyzeFile(QString file, QTextStream* stream)
{
    QFile fileToCheck(file);
    fileToCheck.open(QIODevice::ReadOnly);
    if(fileEntropy(&fileToCheck))
    {
        count++;
        if(fast==0)
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
}

bool fileEntropy(QFile* file)
{
    if(file->size() < 32)
    {
        return false;
    }
    total=0;
    QHash<char, int> data;
    while(!file->atEnd())
    {
        QByteArray read = file->read(1);
        data[read[0]]++;
        total++;
        if(fast == 1 && total >= bytes)
        {
            break;
        }
    }
    QHashIterator<char, int> i(data);
    float entropy=0;
    while (i.hasNext())
    {
        i.next();
        float current = 1.0*i.value()/total;
        entropy -= current*log2(current);
    }
    if(entropy<6)
    {
        return false;
    }
    qDebug() << entropy;
    return true;
}

QString fileLength()
{
   for(int i = 32; i <= 512; i+=32)
   {
       if(total%i == 0)
       {
           return QString::number(i) + " bit multiple";
       }
   }
   return "unknown";
}

void help()
{
    qDebug() << "Usage:\n-m   --mount\t\t\tMount all partitions at \"~/dev\"\n-d   --dir\t--directory\tDirectory to search (\"~/dev\" by default)\n-s   --search\t\t\tSearch for encrypted files\n-o   --output\t\t\tOutput file (\"output.txt\" by default)\n-f   --fast\t\t\tRead only a certain number of bytes from each file\n-b   --bytes\t\t\tNumber of bytes to read when using the fast option (2048 by default)";
}
