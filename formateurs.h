#ifndef FORMATEURS_H
#define FORMATEURS_H
#include <QString>
#include <QDate>
#include <QSqlQueryModel>
class formateurs
{
public:
    formateurs();
    formateurs(int id_formateur, QString nom, QString prenom, QString cin, QString email, QString telephone, QString specialite, QDate date_embauche, QString statut, QString sexe);
    int getId_formateur() const { return id_formateur; }
    void setId_formateur(int value) { id_formateur = value; }
    QString getNom() const { return nom; }
    void setNom(QString value) { nom = value; }
    QString getPrenom() const { return prenom; }
    void setPrenom(QString value) { prenom = value; }
    QString getCin() const { return cin; }
    void setCin(QString value) { cin = value; }
    QString getEmail() const { return email; }
    void setEmail(QString value) { email = value; }
    QString getTelephone() const { return telephone; }
    void setTelephone(QString value) { telephone = value; }
    QString getSpecialite() const { return specialite; }
    void setSpecialite(QString value) { specialite = value; }
    QDate getDate_embauche() const { return date_embauche; }
    void setDate_embauche(QDate value) { date_embauche = value; }
    QString getStatut() const { return statut; }
    void setStatut(QString value) { statut = value; }
    QString getSexe() const { return sexe; }
    void setSexe(QString value) { sexe = value; }
    bool ajouter();
    bool modifier();
    bool supprimer(int id);
    QSqlQueryModel *afficher();
    QSqlQueryModel *chercher(QString column, QString text);
    QSqlQueryModel *tri(QString column, QString choix);
private:
    int id_formateur;
    QString nom;
    QString prenom;
    QString cin;
    QString email;
    QString telephone;
    QString specialite;
    QDate date_embauche;
    QString statut;
    QString sexe;
};
#endif

