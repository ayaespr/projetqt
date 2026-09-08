#ifndef FORMER_H
#define FORMER_H
#include <QSqlQueryModel>
class former
{
public:
    former(int id_stagiaire = 0, int id_formateur = 0);
    bool ajouter();
    bool supprimer();
    QSqlQueryModel *afficher();
private:
    int id_stagiaire;
    int id_formateur;
};
#endif
