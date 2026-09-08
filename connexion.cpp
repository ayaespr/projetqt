#include "connexion.h"
#include <QCoreApplication>
Connexion::Connexion() {}
bool Connexion::createconnexion()
{
    // SQLite is only used when explicitly requested for local tests.
    const bool local = qEnvironmentVariable("FORMATION_SQLITE") == "1";
    QSqlDatabase db = QSqlDatabase::addDatabase(local ? "QSQLITE" : "QODBC");
    db.setDatabaseName(local ? qEnvironmentVariable("FORMATION_DB", "formation.db")
                            : qEnvironmentVariable("FORMATION_DSN", "SOURCE_projet2A"));
    if (!local) {
        db.setUserName(qEnvironmentVariable("FORMATION_USER"));
        db.setPassword(qEnvironmentVariable("FORMATION_PASSWORD"));
    }
    if (!db.open()) return false;
    if (local) {
        QSqlQuery query;
        return query.exec("PRAGMA foreign_keys = ON");
    }
    return true;
}
void Connexion::fermerConnexion()
{
    QSqlDatabase::database().close();
}
