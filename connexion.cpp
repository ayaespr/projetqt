#include "connexion.h"
#include <QSqlError>

Connexion::Connexion()
{
}

bool Connexion::createconnexion()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("projetqt");
    db.setUserName("aya");
    db.setPassword("aya");

    bool opened = db.open();
    if (!opened) {
    }
    return opened;
}

void Connexion::fermerConnexion()
{
    QSqlDatabase::database().close();
}

