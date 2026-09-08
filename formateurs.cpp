#include "formateurs.h"
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

formateurs::formateurs() : id_formateur(0) {}
formateurs::formateurs(int id_formateur, QString nom, QString prenom, QString cin, QString email, QString telephone, QString specialite, QDate date_embauche, QString statut, QString sexe)
{
    this->id_formateur = id_formateur;
    this->nom = nom;
    this->prenom = prenom;
    this->cin = cin;
    this->email = email;
    this->telephone = telephone;
    this->specialite = specialite;
    this->date_embauche = date_embauche;
    this->statut = statut;
    this->sexe = sexe;
}
bool formateurs::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO FORMATEURS (nom, prenom, cin, email, telephone, specialite, date_embauche, statut, sexe) VALUES (:nom, :prenom, :cin, :email, :telephone, :specialite, :date_embauche, :statut, :sexe)");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":cin", cin);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":date_embauche", date_embauche);
    query.bindValue(":statut", statut);
    query.bindValue(":sexe", sexe);
    return query.exec();
}
bool formateurs::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE FORMATEURS SET nom = :nom, prenom = :prenom, cin = :cin, email = :email, telephone = :telephone, specialite = :specialite, date_embauche = :date_embauche, statut = :statut, sexe = :sexe WHERE id_formateur = :id");
    query.bindValue(":id", id_formateur);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":cin", cin);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":date_embauche", date_embauche);
    query.bindValue(":statut", statut);
    query.bindValue(":sexe", sexe);
    return query.exec() && query.numRowsAffected() != 0;
}
bool formateurs::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM FORMATEURS WHERE id_formateur = :id");
    query.bindValue(":id", id);
    return query.exec() && query.numRowsAffected() != 0;
}
QSqlQueryModel *formateurs::afficher()
{
    auto *model = new QSqlQueryModel;
    model->setQuery("SELECT id_formateur, nom, prenom, cin, email, telephone, specialite, date_embauche, statut, sexe FROM FORMATEURS ORDER BY id_formateur");
    return model;
}
QSqlQueryModel *formateurs::chercher(QString column, QString text)
{
    const QStringList columns = {"ID_FORMATEUR", "NOM", "PRENOM", "CIN", "EMAIL", "TELEPHONE", "SPECIALITE", "DATE_EMBAUCHE", "STATUT", "SEXE"};
    if (!columns.contains(column.toUpper())) return afficher();
    QSqlQuery query;
    query.prepare("SELECT id_formateur, nom, prenom, cin, email, telephone, specialite, date_embauche, statut, sexe FROM FORMATEURS WHERE " + column + " LIKE :text");
    query.bindValue(":text", "%" + text + "%");
    query.exec();
    auto *model = new QSqlQueryModel;
    model->setQuery(std::move(query));
    return model;
}
QSqlQueryModel *formateurs::tri(QString column, QString choix)
{
    const QStringList columns = {"ID_FORMATEUR", "NOM", "PRENOM", "CIN", "EMAIL", "TELEPHONE", "SPECIALITE", "DATE_EMBAUCHE", "STATUT", "SEXE"};
    if (!columns.contains(column.toUpper())) return afficher();
    if (choix != "DESC") choix = "ASC";
    auto *model = new QSqlQueryModel;
    model->setQuery("SELECT id_formateur, nom, prenom, cin, email, telephone, specialite, date_embauche, statut, sexe FROM FORMATEURS ORDER BY " + column + " " + choix);
    return model;
}

