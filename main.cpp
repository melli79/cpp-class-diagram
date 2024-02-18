#include "GraphWindow.h"

#include <QApplication>

int main(int nArgs, char** args) {
    QApplication app(nArgs, args);
    GraphWindow w;
    w.show();
    return app.exec();
}
