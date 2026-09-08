#include "former.h"
#include <QSqlQuery>
#include <QVariant>
former::former(int id_stagiaire, int id_formateur)
{
    this->id_stagiaire = id_stagiaire;
    this->id_formateur = id_formateur;
}
bool former::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO FORMER (ID_STAGIAIRE, ID_FORMATEUR) VALUES (:s, :f)");
    query.bindValue(":s", id_stagiaire);
    query.bindValue(":f", id_formateur);
    return query.exec();
}
bool former::supprimer()
{
    QSqlQuery query;
    query.prepare("DELETE FROM FORMER WHERE ID_STAGIAIRE = :s AND ID_FORMATEUR = :f");
    query.bindValue(":s", id_stagiaire);
    query.bindValue(":f", id_formateur);
    return query.exec() && query.numRowsAffected() != 0;
}
QSqlQueryModel *former::afficher()
{
    auto *model = new QSqlQueryModel;
    model->setQuery("SELECT s.ID_STAGIAIRE, f.ID_FORMATEUR, s.NOM AS NOM_STAGIAIRE, "
                    "s.PRENOM AS PRENOM_STAGIAIRE, s.EMAIL AS EMAIL_STAGIAIRE, s.NIVEAU, "
                    "f.NOM AS NOM_FORMATEUR, f.PRENOM AS PRENOM_FORMATEUR, "
                    "f.EMAIL AS EMAIL_FORMATEUR, f.SPECIALITE, f.STATUT "
                    "FROM FORMER r JOIN STAGIAIRES s ON s.ID_STAGIAIRE = r.ID_STAGIAIRE "
                    "JOIN FORMATEURS f ON f.ID_FORMATEUR = r.ID_FORMATEUR "
                    "ORDER BY s.ID_STAGIAIRE, f.ID_FORMATEUR");
    return model;
}
