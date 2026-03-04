#include "SongDialog.h"
#include "ui_SongDialog.h"
#include <QMessageBox>

SongDialog::SongDialog(QWidget *parent, const QString &caption, const QString &lyrics)
    : QDialog(parent),
      ui(new Ui::SongDialog)
{
    ui->setupUi(this);
    
    // Set initial values if provided
    if (!caption.isEmpty()) {
        ui->captionEdit->setPlainText(caption);
    }
    if (!lyrics.isEmpty()) {
        ui->lyricsEdit->setPlainText(lyrics);
    }
}

SongDialog::~SongDialog()
{
    delete ui;
}

QString SongDialog::getCaption() const
{
    return ui->captionEdit->toPlainText();
}

QString SongDialog::getLyrics() const
{
    return ui->lyricsEdit->toPlainText();
}

void SongDialog::on_okButton_clicked()
{
    // Validate that caption is not empty
    QString caption = getCaption();
    if (caption.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Caption cannot be empty.");
        return;
    }
    
    accept();
}

void SongDialog::on_cancelButton_clicked()
{
    reject();
}