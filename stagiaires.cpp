#include "stagiaires.h"
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

stagiaires::stagiaires() : id_stagiaire(0) {}
stagiaires::stagiaires(int id_stagiaire, QString nom, QString prenom, QString email, QString telephone, QString adresse, QDate date_de_naissance, QString sexe, QString cin, QString niveau)
{
    this->id_stagiaire = id_stagiaire;
    this->nom = nom;
    this->prenom = prenom;
    this->email = email;
    this->telephone = telephone;
    this->adresse = adresse;
    this->date_de_naissance = date_de_naissance;
    this->sexe = sexe;
    this->cin = cin;
    this->niveau = niveau;
}
bool stagiaires::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO STAGIAIRES (nom, prenom, email, telephone, adresse, date_de_naissance, sexe, cin, niveau) VALUES (:nom, :prenom, :email, :telephone, :adresse, :date_de_naissance, :sexe, :cin, :niveau)");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":date_de_naissance", date_de_naissance);
    query.bindValue(":sexe", sexe);
    query.bindValue(":cin", cin);
    query.bindValue(":niveau", niveau);
    return query.exec();
}
bool stagiaires::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE STAGIAIRES SET nom = :nom, prenom = :prenom, email = :email, telephone = :telephone, adresse = :adresse, date_de_naissance = :date_de_naissance, sexe = :sexe, cin = :cin, niveau = :niveau WHERE id_stagiaire = :id");
    query.bindValue(":id", id_stagiaire);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":date_de_naissance", date_de_naissance);
    query.bindValue(":sexe", sexe);
    query.bindValue(":cin", cin);
    query.bindValue(":niveau", niveau);
    return query.exec() && query.numRowsAffected() != 0;
}
bool stagiaires::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM STAGIAIRES WHERE id_stagiaire = :id");
    query.bindValue(":id", id);
    return query.exec() && query.numRowsAffected() != 0;
}
QSqlQueryModel *stagiaires::afficher()
{
    auto *model = new QSqlQueryModel;
    model->setQuery("SELECT id_stagiaire, nom, prenom, email, telephone, adresse, date_de_naissance, sexe, cin, niveau FROM STAGIAIRES ORDER BY id_stagiaire");
    return model;
}
QSqlQueryModel *stagiaires::chercher(QString column, QString text)
{
    const QStringList columns = {"ID_STAGIAIRE", "NOM", "PRENOM", "EMAIL", "TELEPHONE", "ADRESSE", "DATE_DE_NAISSANCE", "SEXE", "CIN", "NIVEAU"};
    if (!columns.contains(column.toUpper())) return afficher();
    QSqlQuery query;
    query.prepare("SELECT id_stagiaire, nom, prenom, email, telephone, adresse, date_de_naissance, sexe, cin, niveau FROM STAGIAIRES WHERE " + column + " LIKE :text");
    query.bindValue(":text", "%" + text + "%");
    query.exec();
    auto *model = new QSqlQueryModel;
    model->setQuery(std::move(query));
    return model;
}
QSqlQueryModel *stagiaires::tri(QString column, QString choix)
{
    const QStringList columns = {"ID_STAGIAIRE", "NOM", "PRENOM", "EMAIL", "TELEPHONE", "ADRESSE", "DATE_DE_NAISSANCE", "SEXE", "CIN", "NIVEAU"};
    if (!columns.contains(column.toUpper())) return afficher();
    if (choix != "DESC") choix = "ASC";
    auto *model = new QSqlQueryModel;
    model->setQuery("SELECT id_stagiaire, nom, prenom, email, telephone, adresse, date_de_naissance, sexe, cin, niveau FROM STAGIAIRES ORDER BY " + column + " " + choix);
    return model;
}

