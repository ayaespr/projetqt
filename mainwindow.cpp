#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QTextEdit>
#include <QDialog>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QHeaderView>
#include <QChartView>
#include <QPieSeries>
#include <QChart>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->bt_page_stagiaires, &QPushButton::clicked, this, [this] { ui->stackedWidget->setCurrentIndex(0); });
    on_bt_vider_stagiaire_clicked();
    on_bt_actualiser_stagiaire_clicked();

    ui->stackedWidget->setCurrentIndex(0);
}
MainWindow::~MainWindow() { delete ui; }
void MainWindow::afficherModele(QTableView *table, QSqlQueryModel *model)
{
    if (model->lastError().isValid()) {
        QMessageBox::warning(this, "Base de donnees", model->lastError().text());
        delete model;
        return;
    }
    auto *old = table->model();
    model->setParent(table);
    table->setModel(model);
    delete old;
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
}

bool MainWindow::lirestagiaires(stagiaires &value)
{
    QString nom = ui->nom_stagiaire->text().trimmed();
    QString prenom = ui->prenom_stagiaire->text().trimmed();
    QString email = ui->email_stagiaire->text().trimmed();
    QString telephone = ui->telephone_stagiaire->text().trimmed();
    QString adresse = ui->adresse_stagiaire->text().trimmed();
    QDate date_de_naissance = ui->date_de_naissance_stagiaire->date();
    QString sexe = ui->sexe_stagiaire->currentText();
    QString cin = ui->cin_stagiaire->text().trimmed();
    QString niveau = ui->niveau_stagiaire->text().trimmed();
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty() || telephone.isEmpty() || adresse.isEmpty() || sexe.isEmpty() || cin.isEmpty() || niveau.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Tous les champs sont obligatoires."); return false;
    }
    if (!QRegularExpression("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$").match(email).hasMatch()
        || !QRegularExpression("^[0-9]{8}$").match(cin).hasMatch()
        || !QRegularExpression("^\\+?[0-9 ]{8,20}$").match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Validation", "Verifier email, CIN (8 chiffres) et telephone."); return false;
    }
    if (date_de_naissance >= QDate::currentDate()) {
        QMessageBox::warning(this, "Validation", "Date invalide."); return false;
    }
    value = stagiaires(id_stagiaire, nom, prenom, email, telephone, adresse, date_de_naissance, sexe, cin, niveau);
    return true;
}
void MainWindow::enregistrerstagiaires(bool modification)
{
    if (modification && id_stagiaire == 0) {
        QMessageBox::warning(this, "Selection", "Selectionner une ligne."); return;
    }
    stagiaires value;
    if (!lirestagiaires(value)) return;
    if (!(modification ? value.modifier() : value.ajouter())) {
        QMessageBox::warning(this, "Erreur", "Enregistrement impossible. Verifier les doublons CIN/email et la connexion."); return;
    }
    addToHistory("stagiaires", modification ? "Modification" : "Ajout", modification ? QString::number(id_stagiaire) : value.getCin());
    on_bt_vider_stagiaire_clicked();
    on_bt_actualiser_stagiaire_clicked();
}
void MainWindow::on_bt_ajouter_stagiaire_clicked() { enregistrerstagiaires(false); }
void MainWindow::on_bt_modifier_stagiaire_clicked() { enregistrerstagiaires(true); }
void MainWindow::on_bt_supprimer_stagiaire_clicked()
{
    if (id_stagiaire == 0) { QMessageBox::warning(this, "Selection", "Selectionner une ligne."); return; }
    if (QMessageBox::question(this, "Suppression", "Supprimer cette personne ?") != QMessageBox::Yes) return;
    if (!stagiaire.supprimer(id_stagiaire)) {
        QMessageBox::warning(this, "Erreur", "Suppression impossible. Retirer les affectations existantes avant de supprimer."); return;
    }
    addToHistory("stagiaires", "Suppression", QString::number(id_stagiaire));
    on_bt_vider_stagiaire_clicked();
    on_bt_actualiser_stagiaire_clicked();
}
void MainWindow::on_bt_vider_stagiaire_clicked()
{
    id_stagiaire = 0;
    ui->tableView_stagiaire->clearSelection();
    ui->nom_stagiaire->clear();
    ui->prenom_stagiaire->clear();
    ui->email_stagiaire->clear();
    ui->telephone_stagiaire->clear();
    ui->adresse_stagiaire->clear();
    ui->date_de_naissance_stagiaire->setDate(QDate::currentDate().addYears(-18));
    ui->sexe_stagiaire->setCurrentIndex(0);
    ui->cin_stagiaire->clear();
    ui->niveau_stagiaire->clear();
}
void MainWindow::on_tableView_stagiaire_clicked(const QModelIndex &index)
{
    auto *model = ui->tableView_stagiaire->model();
    id_stagiaire = model->index(index.row(), 0).data().toInt();
    ui->nom_stagiaire->setText(model->index(index.row(), 1).data().toString());
    ui->prenom_stagiaire->setText(model->index(index.row(), 2).data().toString());
    ui->email_stagiaire->setText(model->index(index.row(), 3).data().toString());
    ui->telephone_stagiaire->setText(model->index(index.row(), 4).data().toString());
    ui->adresse_stagiaire->setText(model->index(index.row(), 5).data().toString());
    ui->date_de_naissance_stagiaire->setDate(model->index(index.row(), 6).data().toDate());
    ui->sexe_stagiaire->setCurrentText(model->index(index.row(), 7).data().toString());
    ui->cin_stagiaire->setText(model->index(index.row(), 8).data().toString());
    ui->niveau_stagiaire->setText(model->index(index.row(), 9).data().toString());
}
void MainWindow::on_bt_actualiser_stagiaire_clicked()
{
    on_bt_vider_stagiaire_clicked();
    afficherModele(ui->tableView_stagiaire, stagiaire.afficher());
}
void MainWindow::on_bt_chercher_stagiaire_clicked()
{
    on_bt_vider_stagiaire_clicked();
    afficherModele(ui->tableView_stagiaire, stagiaire.chercher(ui->choix_stagiaire->currentText(), ui->chercher_stagiaire->text().trimmed()));
}
void MainWindow::on_bt_tri_stagiaire_clicked()
{
    on_bt_vider_stagiaire_clicked();
    afficherModele(ui->tableView_stagiaire, stagiaire.tri(ui->choix_stagiaire->currentText(), ui->ordre_stagiaire->currentText()));
}
void MainWindow::on_bt_historique_stagiaire_clicked() { afficherHistorique("stagiaires"); }
void MainWindow::on_bt_pdf_stagiaire_clicked() { exporterPdf(ui->tableView_stagiaire, "stagiaires"); }
void MainWindow::on_bt_stat_stagiaire_clicked() { statistiques("stagiaires"); }

void MainWindow::addToHistory(const QString &table, const QString &action, const QString &id)
{
    QFile file("historique_" + table + ".txt");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Historique", "Operation effectuee, mais historique non enregistre."); return;
    }
    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " | " << action << " | " << id << "\n";
    out.flush();
    if (file.error() != QFileDevice::NoError) QMessageBox::warning(this, "Historique", "Ecriture de l'historique echouee.");
}
void MainWindow::afficherHistorique(const QString &table)
{
    QFile file("historique_" + table + ".txt");
    QString content = "Aucune action enregistree.";
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { QMessageBox::warning(this, "Historique", file.errorString()); return; }
        content = QString::fromUtf8(file.readAll());
    }
    QDialog dialog(this);
    dialog.setWindowTitle("Historique " + table);
    dialog.setFixedSize(720, 450);
    QTextEdit text(&dialog);
    text.setGeometry(10, 10, 700, 430);
    text.setReadOnly(true);
    text.setPlainText(content);
    dialog.exec();
}
void MainWindow::exporterPdf(QTableView *table, const QString &titre)
{
    auto *model = table->model();
    if (!model) return;
    while (model->canFetchMore(QModelIndex())) model->fetchMore(QModelIndex());
    QString path = QFileDialog::getSaveFileName(this, "Exporter PDF", titre + ".pdf", "PDF (*.pdf)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".pdf", Qt::CaseInsensitive)) path += ".pdf";
    QString html = "<h1>" + titre.toHtmlEscaped() + "</h1><table border='1' cellspacing='0' cellpadding='4'><tr>";
    for (int c = 0; c < model->columnCount(); ++c) html += "<th>" + model->headerData(c, Qt::Horizontal).toString().toHtmlEscaped() + "</th>";
    html += "</tr>";
    for (int r = 0; r < model->rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < model->columnCount(); ++c) html += "<td>" + model->index(r,c).data().toString().toHtmlEscaped() + "</td>";
        html += "</tr>";
    }
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(path);
    QTextDocument document;
    document.setHtml(html + "</table>");
    document.print(&printer);
    if (printer.printerState() == QPrinter::Error) QMessageBox::warning(this, "PDF", "Export echoue.");
}
void MainWindow::statistiques(const QString &table)
{
    QSqlQuery query;
    if (!query.exec("SELECT SEXE, COUNT(*) FROM " + table + " GROUP BY SEXE")) {
        QMessageBox::warning(this, "Statistiques", query.lastError().text()); return;
    }
    auto *series = new QPieSeries;
    while (query.next()) series->append(query.value(0).toString(), query.value(1).toInt());
    auto *chart = new QChart;
    chart->addSeries(series);
    chart->setTitle(table + " par sexe");
    series->setLabelsVisible();
    QDialog dialog(this);
    dialog.setFixedSize(600, 400);
    QChartView view(chart, &dialog);
    view.setGeometry(10, 10, 580, 380);
    view.setRenderHint(QPainter::Antialiasing);
    dialog.exec();
}
