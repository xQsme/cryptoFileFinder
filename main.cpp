#include <QCoreApplication>
#include <QtCore>
#include "thread.h"
#include "search.h"

void mountPartitions();
int unmount();
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
    QCommandLineOption unmountOption(QStringList() << "u" << "umount" << "unmount",
                QCoreApplication::translate("main", "Mount all partitions"));
    parser.addOption(unmountOption);
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
    if(args.contains("u") || args.contains("umount") || args.contains("umount"))
    {
        qDebug() << "Unmounting partitions...";
        return unmount();
    }
    if(args.contains("m")  || args.contains("mount"))
    {
        match=1;
        qDebug() << "Mounting partitions...";
        mountPartitions();
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
    int fast=0;
    int bytes=2048;
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
        fast=1;
    }
    int searching=0;
    if(args.contains("s")  || args.contains("search"))
    {
        match=1;
        searching=1;
        Search s;
        s.setStuff(dir, file, fast, bytes);
        s.search();
    }
    if(match == 0){
        help();
    }
    if(searching == 1)
    {
        return a.exec();
    }
    return 0;
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

int unmount()
{
    if (QProcess::execute(QString("/bin/sh") + " ./umount.sh") < 0){
        qDebug() << "Failed to run unmount script.";
        return -1;
    }
    qDebug() << "Unmounted all partitions.";
    return 0;
}

void help()
{
    qDebug() << "Usage:\n-m   --mount\t\t\tMount all partitions at \"~/dev\"" <<
                "\n-u   --umount\t--unmount\tUnmount all partitions at \"~/dev\"" <<
                "\n-d   --dir\t--directory\tDirectory to search (\"~/dev\" by default)" <<
                "\n-s   --search\t\t\tSearch for encrypted files" <<
                "\n-o   --output\t\t\tOutput file (\"output.txt\" by default)" <<
                "\n-f   --fast\t\t\tRead only a certain number of bytes from each file" <<
                "\n-b   --bytes\t\t\tNumber of bytes to read when using the fast option (2048 by default)";
}
