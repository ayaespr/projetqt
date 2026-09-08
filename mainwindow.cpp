#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlError>
#include <QStyle>
#include <memory>
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
#include <QPieSlice>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QLocale>

namespace {
const QRegularExpression nameRegex(QStringLiteral("^[\\p{L}\\p{M}]+(?:[ '’-][\\p{L}\\p{M}]+)*$"));
const QRegularExpression emailRegex(QStringLiteral("^[A-Za-z0-9]+(?:[._%+\\-][A-Za-z0-9]+)*@[A-Za-z0-9]+(?:-[A-Za-z0-9]+)*(?:\\.[A-Za-z0-9]+(?:-[A-Za-z0-9]+)*)*\\.[A-Za-z]{2,}$"));
void selectSavedValue(QComboBox *combo, const QString &value)
{
    if (!value.isEmpty() && combo->findText(value) < 0) combo->addItem(value);
    combo->setCurrentIndex(combo->findText(value));
}
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->niveau_stagiaire->addItems({"Secondaire", "Baccalauréat", "BTS", "Licence", "Master", "Ingénieur", "Doctorat", "Autre"});
    ui->specialite_formateur->addItems({"Informatique", "Réseaux et télécommunications", "Gestion", "Comptabilité", "Marketing", "Langues", "Électronique", "Mécanique", "Autre"});
    ui->niveau_stagiaire->setPlaceholderText("Choisir un niveau");
    ui->specialite_formateur->setPlaceholderText("Choisir une spécialité");
    for (auto *edit : {ui->nom_stagiaire, ui->prenom_stagiaire, ui->nom_formateur, ui->prenom_formateur}) {
        edit->setMaxLength(200);
        edit->setValidator(new QRegularExpressionValidator(nameRegex, edit));
        edit->setToolTip("Lettres, espaces, apostrophes et traits d'union uniquement. Exemple : Amine, Jean-Pierre.");
    }
    for (auto *edit : {ui->cin_stagiaire, ui->telephone_stagiaire, ui->cin_formateur, ui->telephone_formateur}) {
        edit->setMaxLength(8);
        auto *validator = new QIntValidator(0, 99999999, edit);
        QLocale locale(QLocale::C);
        locale.setNumberOptions(QLocale::RejectGroupSeparator);
        validator->setLocale(locale);
        edit->setValidator(validator);
        edit->setPlaceholderText("8 chiffres");
        edit->setToolTip("Exactement 8 chiffres, sans espace. Exemple : 01234567.");
    }
    for (auto *edit : {ui->email_stagiaire, ui->email_formateur}) {
        edit->setMaxLength(254);
        edit->setValidator(new QRegularExpressionValidator(emailRegex, edit));
        edit->setPlaceholderText("nom@exemple.tn");
    }
    ui->adresse_stagiaire->setMaxLength(200);
    ui->adresse_stagiaire->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[\\p{L}\\p{M}0-9 ,./'\\x{2019}#-]{5,200}")), ui->adresse_stagiaire));
    ui->adresse_stagiaire->setToolTip("Adresse : 5 a 200 caracteres, lettres, chiffres et ponctuation usuelle.");
    for (auto *edit : findChildren<QLineEdit *>()) {
        if (!edit->validator()) continue;
        connect(edit, &QLineEdit::textChanged, this, [edit] {
            bool invalid = !edit->text().isEmpty() && !edit->hasAcceptableInput();
            if (edit->maxLength() == 8 && !edit->text().isEmpty())
                invalid = !QRegularExpression("^[0-9]{8}$").match(edit->text()).hasMatch();
            edit->setProperty("invalid", invalid);
            edit->style()->unpolish(edit);
            edit->style()->polish(edit);
        });
    }
    const auto configureSearch = [](QComboBox *choice, QLineEdit *edit) {
        const auto update = [choice, edit] {
            const QValidator *old = edit->validator();
            edit->setValidator(nullptr);
            delete old;
            edit->clear();
            const QString field = choice->currentText();
            const bool numeric = field == "CIN" || field == "TELEPHONE";
            edit->setMaxLength(numeric ? 8 : 200);
            const QString pattern = numeric ? QStringLiteral("[0-9]{0,8}")
                : (field == "NOM" || field == "PRENOM" || field == "SEXE")
                    ? QStringLiteral("[\\p{L}\\p{M} '\\x{2019}-]{0,200}")
                    : QStringLiteral("[^\\p{Cc}]{0,200}");
            edit->setValidator(new QRegularExpressionValidator(QRegularExpression(pattern), edit));
            edit->setPlaceholderText(numeric ? "Chiffres uniquement" : "Rechercher...");
        };
        QObject::connect(choice, &QComboBox::currentTextChanged, edit, update);
        update();
    };
    configureSearch(ui->choix_stagiaire, ui->chercher_stagiaire);
    configureSearch(ui->choix_formateur, ui->chercher_formateur);
    ui->date_de_naissance_stagiaire->setMaximumDate(QDate::currentDate().addDays(-1));
    ui->date_embauche_formateur->setMaximumDate(QDate::currentDate());
    connect(ui->bt_page_stagiaires, &QPushButton::clicked, this, [this] { ui->stackedWidget->setCurrentIndex(0); });
    viderFormulaireStagiaire();
    on_bt_actualiser_stagiaire_clicked();
    connect(ui->bt_page_formateurs, &QPushButton::clicked, this, [this] { ui->stackedWidget->setCurrentIndex(1); });
    viderFormulaireFormateur();
    on_bt_actualiser_formateur_clicked();
    connect(ui->bt_page_former, &QPushButton::clicked, this, [this] { on_bt_actualiser_former_clicked(); ui->stackedWidget->setCurrentIndex(2); });
    on_bt_actualiser_former_clicked();
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
    QString niveau = ui->niveau_stagiaire->currentText().trimmed();
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty() || telephone.isEmpty() || adresse.isEmpty() || sexe.isEmpty() || cin.isEmpty() || niveau.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Tous les champs sont obligatoires."); return false;
    }
    if (!nameRegex.match(nom).hasMatch() || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Saisie invalide", "Nom et prénom : lettres uniquement, avec espaces, apostrophes ou traits d'union. Exemple : Jean-Pierre Ben Ali."); return false;
    }
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Saisie invalide", "Adresse email invalide. Format attendu : nom@exemple.tn (exemple : aa@aa.aa)."); return false;
    }
    if (!QRegularExpression("^[0-9]{8}$").match(cin).hasMatch()
        || !QRegularExpression("^[0-9]{8}$").match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Saisie invalide", "Le CIN et le téléphone doivent contenir exactement 8 chiffres, sans espace ni signe. Exemple : 01234567."); return false;
    }
    if (adresse.size() < 5 || !ui->adresse_stagiaire->hasAcceptableInput()) {
        QMessageBox::warning(this, "Saisie invalide", "Adresse : 5 a 200 caracteres, sans caracteres de controle.");
        ui->adresse_stagiaire->setFocus();
        return false;
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
        QMessageBox::warning(this, "Échec", "Enregistrement échoué. Verifier les doublons CIN/email et la connexion."); return;
    }
    addToHistory("stagiaires", modification ? "Modification" : "Ajout", modification ? QString::number(id_stagiaire) : value.getCin());
    viderFormulaireStagiaire();
    on_bt_actualiser_stagiaire_clicked();
    QMessageBox::information(this, "Succès", modification ? "Le stagiaire a été modifié avec succès." : "Le stagiaire a été ajouté avec succès.");
}
void MainWindow::on_bt_ajouter_stagiaire_clicked() { enregistrerstagiaires(false); }
void MainWindow::on_bt_modifier_stagiaire_clicked() { enregistrerstagiaires(true); }
void MainWindow::on_bt_supprimer_stagiaire_clicked()
{
    if (id_stagiaire == 0) { QMessageBox::warning(this, "Selection", "Selectionner une ligne."); return; }
    if (QMessageBox::question(this, "Suppression", "Supprimer cette personne ?") != QMessageBox::Yes) return;
    if (!stagiaire.supprimer(id_stagiaire)) {
        QMessageBox::warning(this, "Échec", "Suppression échouée. Retirer les affectations existantes avant de supprimer."); return;
    }
    addToHistory("stagiaires", "Suppression", QString::number(id_stagiaire));
    viderFormulaireStagiaire();
    on_bt_actualiser_stagiaire_clicked();
    QMessageBox::information(this, "Succès", "Le stagiaire a été supprimé avec succès.");
}
void MainWindow::viderFormulaireStagiaire()
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
    ui->niveau_stagiaire->setCurrentIndex(-1);
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
    selectSavedValue(ui->niveau_stagiaire, model->index(index.row(), 9).data().toString());
}
void MainWindow::on_bt_actualiser_stagiaire_clicked()
{
    viderFormulaireStagiaire();
    afficherModele(ui->tableView_stagiaire, stagiaire.afficher());
}
void MainWindow::on_bt_chercher_stagiaire_clicked()
{
    viderFormulaireStagiaire();
    afficherModele(ui->tableView_stagiaire, stagiaire.chercher(ui->choix_stagiaire->currentText(), ui->chercher_stagiaire->text().trimmed()));
}
void MainWindow::on_bt_tri_stagiaire_clicked()
{
    viderFormulaireStagiaire();
    afficherModele(ui->tableView_stagiaire, stagiaire.tri(ui->choix_stagiaire->currentText(), ui->ordre_stagiaire->currentText()));
}
void MainWindow::on_bt_historique_stagiaire_clicked() { afficherHistorique("stagiaires"); }
void MainWindow::on_bt_pdf_stagiaire_clicked() { exporterPdf(ui->tableView_stagiaire, "stagiaires"); }
void MainWindow::on_bt_stat_stagiaire_clicked() { afficherStatistiques(stagiaire.statistiquesParSexe(), "Statistiques des stagiaires par sexe"); }

bool MainWindow::lireformateurs(formateurs &value)
{
    QString nom = ui->nom_formateur->text().trimmed();
    QString prenom = ui->prenom_formateur->text().trimmed();
    QString cin = ui->cin_formateur->text().trimmed();
    QString email = ui->email_formateur->text().trimmed();
    QString telephone = ui->telephone_formateur->text().trimmed();
    QString specialite = ui->specialite_formateur->currentText().trimmed();
    QDate date_embauche = ui->date_embauche_formateur->date();
    QString statut = ui->statut_formateur->currentText();
    QString sexe = ui->sexe_formateur->currentText();
    if (nom.isEmpty() || prenom.isEmpty() || cin.isEmpty() || email.isEmpty() || telephone.isEmpty() || specialite.isEmpty() || statut.isEmpty() || sexe.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Tous les champs sont obligatoires."); return false;
    }
    if (!nameRegex.match(nom).hasMatch() || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Saisie invalide", "Nom et prénom : lettres uniquement, avec espaces, apostrophes ou traits d'union. Exemple : Jean-Pierre Ben Ali."); return false;
    }
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Saisie invalide", "Adresse email invalide. Format attendu : nom@exemple.tn (exemple : aa@aa.aa)."); return false;
    }
    if (!QRegularExpression("^[0-9]{8}$").match(cin).hasMatch()
        || !QRegularExpression("^[0-9]{8}$").match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Saisie invalide", "Le CIN et le téléphone doivent contenir exactement 8 chiffres, sans espace ni signe. Exemple : 01234567."); return false;
    }
    if (date_embauche > QDate::currentDate()) {
        QMessageBox::warning(this, "Validation", "Date invalide."); return false;
    }
    value = formateurs(id_formateur, nom, prenom, cin, email, telephone, specialite, date_embauche, statut, sexe);
    return true;
}
void MainWindow::enregistrerformateurs(bool modification)
{
    if (modification && id_formateur == 0) {
        QMessageBox::warning(this, "Selection", "Selectionner une ligne."); return;
    }
    formateurs value;
    if (!lireformateurs(value)) return;
    if (!(modification ? value.modifier() : value.ajouter())) {
        QMessageBox::warning(this, "Échec", "Enregistrement échoué. Verifier les doublons CIN/email et la connexion."); return;
    }
    addToHistory("formateurs", modification ? "Modification" : "Ajout", modification ? QString::number(id_formateur) : value.getCin());
    viderFormulaireFormateur();
    on_bt_actualiser_formateur_clicked();
    QMessageBox::information(this, "Succès", modification ? "Le formateur a été modifié avec succès." : "Le formateur a été ajouté avec succès.");
}
void MainWindow::on_bt_ajouter_formateur_clicked() { enregistrerformateurs(false); }
void MainWindow::on_bt_modifier_formateur_clicked() { enregistrerformateurs(true); }
void MainWindow::on_bt_supprimer_formateur_clicked()
{
    if (id_formateur == 0) { QMessageBox::warning(this, "Selection", "Selectionner une ligne."); return; }
    if (QMessageBox::question(this, "Suppression", "Supprimer cette personne ?") != QMessageBox::Yes) return;
    if (!formateur.supprimer(id_formateur)) {
        QMessageBox::warning(this, "Échec", "Suppression échouée. Retirer les affectations existantes avant de supprimer."); return;
    }
    addToHistory("formateurs", "Suppression", QString::number(id_formateur));
    viderFormulaireFormateur();
    on_bt_actualiser_formateur_clicked();
    QMessageBox::information(this, "Succès", "Le formateur a été supprimé avec succès.");
}
void MainWindow::viderFormulaireFormateur()
{
    id_formateur = 0;
    ui->tableView_formateur->clearSelection();
    ui->nom_formateur->clear();
    ui->prenom_formateur->clear();
    ui->cin_formateur->clear();
    ui->email_formateur->clear();
    ui->telephone_formateur->clear();
    ui->specialite_formateur->setCurrentIndex(-1);
    ui->date_embauche_formateur->setDate(QDate::currentDate());
    ui->statut_formateur->setCurrentIndex(0);
    ui->sexe_formateur->setCurrentIndex(0);
}
void MainWindow::on_tableView_formateur_clicked(const QModelIndex &index)
{
    auto *model = ui->tableView_formateur->model();
    id_formateur = model->index(index.row(), 0).data().toInt();
    ui->nom_formateur->setText(model->index(index.row(), 1).data().toString());
    ui->prenom_formateur->setText(model->index(index.row(), 2).data().toString());
    ui->cin_formateur->setText(model->index(index.row(), 3).data().toString());
    ui->email_formateur->setText(model->index(index.row(), 4).data().toString());
    ui->telephone_formateur->setText(model->index(index.row(), 5).data().toString());
    selectSavedValue(ui->specialite_formateur, model->index(index.row(), 6).data().toString());
    ui->date_embauche_formateur->setDate(model->index(index.row(), 7).data().toDate());
    ui->statut_formateur->setCurrentText(model->index(index.row(), 8).data().toString());
    ui->sexe_formateur->setCurrentText(model->index(index.row(), 9).data().toString());
}
void MainWindow::on_bt_actualiser_formateur_clicked()
{
    viderFormulaireFormateur();
    afficherModele(ui->tableView_formateur, formateur.afficher());
}
void MainWindow::on_bt_chercher_formateur_clicked()
{
    viderFormulaireFormateur();
    afficherModele(ui->tableView_formateur, formateur.chercher(ui->choix_formateur->currentText(), ui->chercher_formateur->text().trimmed()));
}
void MainWindow::on_bt_tri_formateur_clicked()
{
    viderFormulaireFormateur();
    afficherModele(ui->tableView_formateur, formateur.tri(ui->choix_formateur->currentText(), ui->ordre_formateur->currentText()));
}
void MainWindow::on_bt_historique_formateur_clicked() { afficherHistorique("formateurs"); }
void MainWindow::on_bt_pdf_formateur_clicked() { exporterPdf(ui->tableView_formateur, "formateurs"); }
void MainWindow::on_bt_stat_formateur_clicked() { afficherStatistiques(formateur.statistiquesParSexe(), "Statistiques des formateurs par sexe"); }

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
    if (printer.printerState() == QPrinter::Error || !QFile::exists(path) || QFile(path).size() == 0) QMessageBox::warning(this, "Échec", "Export PDF échoué. Vérifiez le dossier et les droits d’écriture.");
    else QMessageBox::information(this, "Succès", "Le PDF a été exporté avec succès :\n" + path);
}
void MainWindow::afficherStatistiques(const QMap<QString, int> &stats, const QString &titre)
{
    int total = 0;
    for (int count : stats) total += count;
    if (total <= 0) {
        QMessageBox::information(this, "Statistiques", "Aucune donnée disponible pour calculer les pourcentages.");
        return;
    }
    auto *series = new QPieSeries;
    for (auto it = stats.cbegin(); it != stats.cend(); ++it) {
        if (it.value() <= 0) continue;
        auto *slice = series->append(QString("%1 : %2 (%3 %)").arg(it.key()).arg(it.value()).arg(100.0 * it.value() / total, 0, 'f', 1), it.value());
        slice->setLabelColor(QColor("#172b3a"));
    }
    auto *chart = new QChart;
    chart->addSeries(series);
    chart->setTitle(titre + QString(" — Total : %1").arg(total));
    chart->setTheme(QChart::ChartThemeLight);
    series->setLabelsVisible();
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques");
    auto *view = new QChartView(chart, &dialog);
    view->setRenderHint(QPainter::Antialiasing);
    dialog.setFixedSize(820, 540);
    view->setGeometry(10, 10, 800, 520);
    dialog.exec();
}

void MainWindow::on_bt_actualiser_former_clicked()
{
    const auto remplirListe = [this](QComboBox *liste, QSqlQueryModel *resultat, const QString &titre) {
        std::unique_ptr<QSqlQueryModel> model(resultat);
        liste->clear();
        if (model->lastError().isValid()) {
            QMessageBox::warning(this, titre, model->lastError().text());
            return;
        }
        while (model->canFetchMore(QModelIndex())) model->fetchMore(QModelIndex());
        for (int row = 0; row < model->rowCount(); ++row) {
            const QVariant id = model->index(row, 0).data();
            const QString nom = model->index(row, 1).data().toString();
            const QString prenom = model->index(row, 2).data().toString();
            liste->addItem(nom + " " + prenom + " (#" + id.toString() + ")", id);
        }
    };
    remplirListe(ui->stagiaire_former, stagiaire.listePourAffectation(), "Stagiaires");
    remplirListe(ui->formateur_former, formateur.listePourAffectation(), "Formateurs");
    former relation;
    afficherModele(ui->tableView_former, relation.afficher());
}
void MainWindow::on_bt_affecter_clicked()
{
    int s = ui->stagiaire_former->currentData().toInt();
    int f = ui->formateur_former->currentData().toInt();
    if (s == 0 || f == 0) { QMessageBox::warning(this, "Affectation", "Choisir un stagiaire et un formateur."); return; }
    former relation(s, f);
    if (!relation.ajouter()) { QMessageBox::warning(this, "Affectation", "Affectation impossible : doublon, personne supprimee ou connexion indisponible."); return; }
    addToHistory("stagiaires", "Affectation au formateur " + QString::number(f), QString::number(s));
    addToHistory("formateurs", "Affectation du stagiaire " + QString::number(s), QString::number(f));
    on_bt_actualiser_former_clicked();
    QMessageBox::information(this, "Succès", "Le stagiaire a été affecté au formateur avec succès.");
}
void MainWindow::on_bt_desaffecter_clicked()
{
    QModelIndex index = ui->tableView_former->currentIndex();
    if (!index.isValid()) { QMessageBox::warning(this, "Selection", "Selectionner une affectation dans le tableau."); return; }
    auto *model = ui->tableView_former->model();
    int s = model->index(index.row(), 0).data().toInt();
    int f = model->index(index.row(), 1).data().toInt();
    if (QMessageBox::question(this, "Affectation", "Retirer cette affectation ?") != QMessageBox::Yes) return;
    former relation(s, f);
    if (!relation.supprimer()) { QMessageBox::warning(this, "Affectation", "Suppression impossible."); return; }
    addToHistory("stagiaires", "Retrait du formateur " + QString::number(f), QString::number(s));
    addToHistory("formateurs", "Retrait du stagiaire " + QString::number(s), QString::number(f));
    on_bt_actualiser_former_clicked();
    QMessageBox::information(this, "Succès", "L’affectation a été retirée avec succès.");
}

