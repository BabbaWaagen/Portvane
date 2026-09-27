#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // QSettings uses these two names to decide where settings are stored.
    QApplication::setOrganizationName(QStringLiteral("Portvane"));
    QApplication::setApplicationName(QStringLiteral("Portvane"));

    MainWindow window;
    window.show();

    return app.exec();
}
