#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTableView>
#include <QSqlQueryModel>
#include "stagiaires.h"

namespace Ui { class MainWindow; }
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_bt_ajouter_stagiaire_clicked();
    void on_bt_modifier_stagiaire_clicked();
    void on_bt_supprimer_stagiaire_clicked();
    void on_bt_vider_stagiaire_clicked();
    void on_bt_historique_stagiaire_clicked();
    void on_bt_pdf_stagiaire_clicked();
    void on_bt_stat_stagiaire_clicked();
    void on_bt_chercher_stagiaire_clicked();
    void on_bt_tri_stagiaire_clicked();
    void on_bt_actualiser_stagiaire_clicked();
    void on_tableView_stagiaire_clicked(const QModelIndex &index);
private:
    Ui::MainWindow *ui;
    void afficherModele(QTableView *table, QSqlQueryModel *model);
    void addToHistory(const QString &table, const QString &action, const QString &id);
    void afficherHistorique(const QString &table);
    void exporterPdf(QTableView *table, const QString &titre);
    void statistiques(const QString &table);
    stagiaires stagiaire;
    int id_stagiaire = 0;
    bool lirestagiaires(stagiaires &value);
    void enregistrerstagiaires(bool modification);
};
#endif
