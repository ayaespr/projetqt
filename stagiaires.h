#ifndef STAGIAIRES_H
#define STAGIAIRES_H
#include <QString>
#include <QMap>
#include <QDate>
#include <QSqlQueryModel>
class stagiaires
{
public:
    stagiaires();
    stagiaires(int id_stagiaire, QString nom, QString prenom, QString email, QString telephone, QString adresse, QDate date_de_naissance, QString sexe, QString cin, QString niveau);
    int getId_stagiaire() const { return id_stagiaire; }
    void setId_stagiaire(int value) { id_stagiaire = value; }
    QString getNom() const { return nom; }
    void setNom(QString value) { nom = value; }
    QString getPrenom() const { return prenom; }
    void setPrenom(QString value) { prenom = value; }
    QString getEmail() const { return email; }
    void setEmail(QString value) { email = value; }
    QString getTelephone() const { return telephone; }
    void setTelephone(QString value) { telephone = value; }
    QString getAdresse() const { return adresse; }
    void setAdresse(QString value) { adresse = value; }
    QDate getDate_de_naissance() const { return date_de_naissance; }
    void setDate_de_naissance(QDate value) { date_de_naissance = value; }
    QString getSexe() const { return sexe; }
    void setSexe(QString value) { sexe = value; }
    QString getCin() const { return cin; }
    void setCin(QString value) { cin = value; }
    QString getNiveau() const { return niveau; }
    void setNiveau(QString value) { niveau = value; }
    bool ajouter();
    bool modifier();
    bool supprimer(int id);
    QSqlQueryModel *afficher();
    QSqlQueryModel *listePourAffectation();
    QMap<QString, int> statistiquesParSexe();
    QSqlQueryModel *chercher(QString column, QString text);
    QSqlQueryModel *tri(QString column, QString choix);
private:
    int id_stagiaire;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString adresse;
    QDate date_de_naissance;
    QString sexe;
    QString cin;
    QString niveau;
};
#endif
