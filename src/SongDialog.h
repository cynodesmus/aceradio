/*
 * Copyright Carl Philipp Klemm, cynodesmus 2026
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SONGDIALOG_H
#define SONGDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class SongDialog;
}

class SongDialog : public QDialog {
    Q_OBJECT

  public:
    explicit SongDialog(QWidget *       parent        = nullptr,
                        const QString & caption       = "",
                        const QString & lyrics        = "",
                        const QString & vocalLanguage = "",
                        bool            cotEnabled    = true);
    ~SongDialog();

    QString getCaption() const;
    QString getLyrics() const;
    QString getVocalLanguage() const;
    bool    getCotEnabled() const;

  private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

  private:
    Ui::SongDialog * ui;
};

#endif  // SONGDIALOG_H
