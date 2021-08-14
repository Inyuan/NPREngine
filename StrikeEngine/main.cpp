#include "StrikeEngine.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    wchar_t* path = new wchar_t[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, path);
    printf("%s\n", path);

    QApplication a(argc, argv);
    StrikeEngine w;
    w.show();
    return a.exec();
}
