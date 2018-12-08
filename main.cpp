#include <QtCore>
#include "thread.h"
#include "search.h"

void mountPartitions();
int unmount();
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
    QCommandLineOption testOption(QStringList() << "t" << "test" << "testing",
                QCoreApplication::translate("main", "Generate test data"));
    parser.addOption(testOption);
    QCommandLineOption searchOption(QStringList() << "s" << "search",
                QCoreApplication::translate("main", "Search for encrypted files"));
    parser.addOption(searchOption);
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
        if(args.contains("t") || args.contains("test") || args.contains("testing"))
        {
            file="output.csv";
        }
        else
        {
            file="output.txt";
        }
    }
    int testing=0;
    if(args.contains("t") || args.contains("test") || args.contains("testing"))
    {
        testing=1;
    }
    if(args.contains("s")  || args.contains("search"))
    {
        int toUnmount=0;
        if(args.contains("u") || args.contains("umount") || args.contains("umount"))
        {
            toUnmount=1;
        }
        match=1;
        Search s;
        s.setStuff(dir, file, toUnmount, &a, testing);
        return s.search();
    }
    else
    {
        if(args.contains("u") || args.contains("umount") || args.contains("umount"))
        {
            qDebug() << "Unmounting partitions...";
            return unmount();
        }
    }
    if(!match){
        help();
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
    qDebug() << "Done unmounting.";
    return 0;
}

void help()
{
    qDebug() << "Usage:\n-m   --mount\t\t\tMount all partitions at \"~/dev\"" <<
                "\n-u   --umount\t--unmount\tUnmount all partitions at \"~/dev\"" <<
                "\n-d   --dir\t--directory\tDirectory to search (\"~/dev\" by default)" <<
                "\n-s   --search\t\t\tSearch for encrypted files" <<
                "\n-o   --output\t\t\tOutput file (\"output.txt\" by default)" <<
                "\n-t   --test\t--testing\tGenerate csv test data (statistical values for every file)";
}
