#include "mainwindow.h"
#include "connexion.h"
#include <QApplication>
#include <QMessageBox>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("projet_aya_septembre");
    app.setStyleSheet("QMessageBox { background: #f5f7fa; color: black; } QMessageBox QLabel { color: black; } QMessageBox QPushButton { background: #e8eef3; color: black; min-width: 80px; padding: 6px; }");
    Connexion connexion;
    if (!connexion.createconnexion()) {
        QMessageBox::critical(nullptr, "Connexion", "Connexion impossible : " + QSqlDatabase::database().lastError().text());
        //return 1;
    }
    MainWindow window;
    window.show();
    return app.exec();
}
