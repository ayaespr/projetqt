#include "mainwindow.h"
#include "connexion.h"
#include <QApplication>
#include <QMessageBox>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("GestionFormation");
    Connexion connexion;
    if (!connexion.createconnexion()) {
        QMessageBox::critical(nullptr, "Connexion", "Connexion impossible : " + QSqlDatabase::database().lastError().text());
        return 1;
    }
    MainWindow window;
    window.show();
    return app.exec();
}
