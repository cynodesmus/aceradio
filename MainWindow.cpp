#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "SongDialog.h"
#include "AdvancedSettingsDialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      songModel(new SongListModel(this)),
      audioPlayer(new AudioPlayer(this)),
      aceStepWorker(new AceStepWorker(this)),
      playbackTimer(new QTimer(this)),
      currentSongIndex(-1),
      isPlaying(false),
      isPaused(false),
      shuffleMode(false),
      isGeneratingNext(false)
{
    ui->setupUi(this);
    
    // Setup UI
    setupUI();
    
    // Load settings
    loadSettings();
    
    // Connect signals and slots
    connect(ui->actionAdvancedSettings, &QAction::triggered, this, &MainWindow::on_advancedSettingsButton_clicked);
    connect(audioPlayer, &AudioPlayer::playbackFinished, this, &MainWindow::playNextSong);
    connect(audioPlayer, &AudioPlayer::playbackStarted, this, &MainWindow::playbackStarted);
    connect(aceStepWorker, &AceStepWorker::songGenerated, this, &MainWindow::songGenerated);
    connect(aceStepWorker, &AceStepWorker::generationError, this, &MainWindow::generationError);
    connect(aceStepWorker, &AceStepWorker::progressUpdate, ui->progressBar, &QProgressBar::setValue);
    
    // Connect audio player error signal
    connect(audioPlayer, &AudioPlayer::playbackError, [this](const QString &error) {
        QMessageBox::warning(this, "Playback Error", "Failed to play audio: " + error);
    });
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::setupUI()
{
    // Setup song list view
    ui->songListView->setModel(songModel);
    
    // Add some default songs
    SongItem defaultSong1("Upbeat pop rock anthem with driving electric guitars", "");
    SongItem defaultSong2("Chill electronic music with smooth synths and relaxing beats", "");
    SongItem defaultSong3("Jazz fusion with saxophone solos and complex rhythms", "");
    
    songModel->addSong(defaultSong1);
    songModel->addSong(defaultSong2);
    songModel->addSong(defaultSong3);
    
    // Select first item
    if (songModel->rowCount() > 0) {
        QModelIndex firstIndex = songModel->index(0, 0);
        ui->songListView->setCurrentIndex(firstIndex);
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("MusicGenerator", "AceStepGUI");
    
    // Load JSON template (default to simple configuration)
    jsonTemplate = settings.value("jsonTemplate", 
        "{\"inference_steps\": 8, \"shift\": 3.0, \"vocal_language\": \"en\"}").toString();
    
    // Load shuffle mode
    shuffleMode = settings.value("shuffleMode", false).toBool();
    ui->shuffleButton->setChecked(shuffleMode);
    
    // Load path settings with defaults based on application directory
    QString appDir = QCoreApplication::applicationDirPath();
    aceStepPath = settings.value("aceStepPath", appDir + "/acestep.cpp").toString();
    qwen3ModelPath = settings.value("qwen3ModelPath", appDir + "/acestep.cpp/models/acestep-5Hz-lm-4B-Q8_0.gguf").toString();
    textEncoderModelPath = settings.value("textEncoderModelPath", appDir + "/acestep.cpp/models/Qwen3-Embedding-0.6B-BF16.gguf").toString();
    ditModelPath = settings.value("ditModelPath", appDir + "/acestep.cpp/models/acestep-v15-turbo-Q8_0.gguf").toString();
    vaeModelPath = settings.value("vaeModelPath", appDir + "/acestep.cpp/models/vae-BF16.gguf").toString();
}

void MainWindow::saveSettings()
{
    QSettings settings("MusicGenerator", "AceStepGUI");
    
    // Save JSON template
    settings.setValue("jsonTemplate", jsonTemplate);
    
    // Save shuffle mode
    settings.setValue("shuffleMode", shuffleMode);
    
    // Save path settings
    settings.setValue("aceStepPath", aceStepPath);
    settings.setValue("qwen3ModelPath", qwen3ModelPath);
    settings.setValue("textEncoderModelPath", textEncoderModelPath);
    settings.setValue("ditModelPath", ditModelPath);
    settings.setValue("vaeModelPath", vaeModelPath);
}

QString MainWindow::formatTime(int milliseconds)
{
    if (milliseconds < 0) return "0:00";
    
    int seconds = milliseconds / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

void MainWindow::updatePosition(int position)
{
    if (position < 0) return;
    
    // Update slider and time labels
    ui->positionSlider->setValue(position);
    ui->elapsedTimeLabel->setText(formatTime(position));
}

void MainWindow::updateDuration(int duration)
{
    if (duration <= 0) return;
    
    // Set slider range and update duration label
    ui->positionSlider->setRange(0, duration);
    ui->durationLabel->setText(formatTime(duration));
}

void MainWindow::updateControls()
{
    bool hasSongs = songModel->rowCount() > 0;
    
    // Play button is enabled when not playing, or can be used to resume when paused
    ui->playButton->setEnabled(hasSongs && (!isPlaying || isPaused));
    ui->pauseButton->setEnabled(isPlaying && !isPaused);
    ui->skipButton->setEnabled(isPlaying);
    ui->stopButton->setEnabled(isPlaying);
    ui->addSongButton->setEnabled(true);
    ui->editSongButton->setEnabled(hasSongs && ui->songListView->currentIndex().isValid());
    ui->removeSongButton->setEnabled(hasSongs && ui->songListView->currentIndex().isValid());
}

void MainWindow::on_playButton_clicked()
{
    if (isPaused) {
        // Resume playback
        audioPlayer->play();
        isPaused = false;
        updateControls();
        return;
    }
    
    if (isPlaying) {
        audioPlayer->stop();
        isPlaying = false;
        isPaused = false;
        updateControls();
        return;
    }
    
    // Start playback from current song or first song
    int startIndex = ui->songListView->currentIndex().isValid() 
                     ? ui->songListView->currentIndex().row() 
                     : 0;
    
    currentSongIndex = startIndex;
    generateAndPlayNext();
}

void MainWindow::on_pauseButton_clicked()
{
    if (isPlaying && !isPaused) {
        // Pause playback
        audioPlayer->pause();
        isPaused = true;
        updateControls();
    }
}

void MainWindow::on_skipButton_clicked()
{
    if (isPlaying) {
        // Stop current playback and move to next song
        audioPlayer->stop();
        isPaused = false;
        playNextSong();
        
        // After playing the skipped-to song, start generating the next one
        // We'll do this in playNextSong by checking if we're already playing
    }
}

void MainWindow::on_stopButton_clicked()
{
    if (isPlaying) {
        // Stop current playback completely
        audioPlayer->stop();
        isPlaying = false;
        isPaused = false;
        ui->statusLabel->setText("Stopped");
        updateControls();
    }
}

void MainWindow::on_shuffleButton_clicked()
{
    shuffleMode = ui->shuffleButton->isChecked();
    updateControls();
}

void MainWindow::on_addSongButton_clicked()
{
    SongDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString caption = dialog.getCaption();
        QString lyrics = dialog.getLyrics();
        
        SongItem newSong(caption, lyrics);
        songModel->addSong(newSong);
        
        // Select the new item
        QModelIndex newIndex = songModel->index(songModel->rowCount() - 1, 0);
        ui->songListView->setCurrentIndex(newIndex);
    }
}

void MainWindow::on_editSongButton_clicked()
{
    QModelIndex currentIndex = ui->songListView->currentIndex();
    if (!currentIndex.isValid()) return;
    
    int row = currentIndex.row();
    SongItem song = songModel->getSong(row);
    
    SongDialog dialog(this, song.caption, song.lyrics);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString caption = dialog.getCaption();
        QString lyrics = dialog.getLyrics();
        
        // Update the model
        songModel->setData(songModel->index(row, 0), caption, SongListModel::CaptionRole);
        songModel->setData(songModel->index(row, 0), lyrics, SongListModel::LyricsRole);
    }
}

void MainWindow::on_removeSongButton_clicked()
{
    QModelIndex currentIndex = ui->songListView->currentIndex();
    if (!currentIndex.isValid()) return;
    
    int row = currentIndex.row();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove Song", "Are you sure you want to remove this song?",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        songModel->removeSong(row);
        
        // Select next item or previous if at end
        int newRow = qMin(row, songModel->rowCount() - 1);
        if (newRow >= 0) {
            QModelIndex newIndex = songModel->index(newRow, 0);
            ui->songListView->setCurrentIndex(newIndex);
        }
    }
}

void MainWindow::on_advancedSettingsButton_clicked()
{
    AdvancedSettingsDialog dialog(this);
    
    // Set current values
    dialog.setJsonTemplate(jsonTemplate);
    dialog.setAceStepPath(aceStepPath);
    dialog.setQwen3ModelPath(qwen3ModelPath);
    dialog.setTextEncoderModelPath(textEncoderModelPath);
    dialog.setDiTModelPath(ditModelPath);
    dialog.setVAEModelPath(vaeModelPath);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Validate JSON template
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(dialog.getJsonTemplate().toUtf8(), &parseError);
        if (!doc.isObject()) {
            QMessageBox::warning(this, "Invalid JSON", "Please enter valid JSON: " + QString(parseError.errorString()));
            return;
        }
        
        // Update settings
        jsonTemplate = dialog.getJsonTemplate();
        aceStepPath = dialog.getAceStepPath();
        qwen3ModelPath = dialog.getQwen3ModelPath();
        textEncoderModelPath = dialog.getTextEncoderModelPath();
        ditModelPath = dialog.getDiTModelPath();
        vaeModelPath = dialog.getVAEModelPath();
        
        saveSettings();
        QMessageBox::information(this, "Settings Saved", "Advanced settings have been saved successfully.");
    }
}

void MainWindow::generateAndPlayNext()
{
    if (currentSongIndex < 0 || currentSongIndex >= songModel->rowCount()) {
        return;
    }
    
    SongItem song = songModel->getSong(currentSongIndex);
    
    // Show status
    ui->statusLabel->setText("Generating: " + song.caption);
    isPlaying = true;
    isGeneratingNext = false; // Reset the flag when starting a new generation
    updateControls();
    
    // Generate the song with configurable paths
    aceStepWorker->generateSong(song.caption, song.lyrics, jsonTemplate,
                               aceStepPath, qwen3ModelPath,
                               textEncoderModelPath, ditModelPath,
                               vaeModelPath);
}

void MainWindow::startNextSongGeneration()
{
    // Start generating the next song if we're playing and not already generating
    if (isPlaying && !isGeneratingNext) {
        isGeneratingNext = true;
        
        // Find and generate the next song
        int nextIndex = songModel->findNextIndex(currentSongIndex, shuffleMode);
        if (nextIndex >= 0 && nextIndex < songModel->rowCount()) {
            SongItem nextSong = songModel->getSong(nextIndex);
            
            // Generate the next song in the background
            aceStepWorker->generateSong(nextSong.caption, nextSong.lyrics, jsonTemplate,
                                       aceStepPath, qwen3ModelPath,
                                       textEncoderModelPath, ditModelPath,
                                       vaeModelPath);
        }
    }
}

void MainWindow::playbackStarted()
{
    // When playback starts, immediately start generating the next song
    startNextSongGeneration();
}

void MainWindow::songGenerated(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        generationError("Generated file not found: " + filePath);
        return;
    }
    
    // If we're in the middle of playback, this is a pre-generated next song
    if (isPlaying && audioPlayer->isPlaying()) {
        // Store the generated file path for when playback finishes
        nextSongFilePath = filePath;
        return;
    }
    
    ui->statusLabel->setText("Playing: " + QFileInfo(filePath).baseName());
    
    // Play the generated song
    audioPlayer->play(filePath);
    
    // Connect position and duration updates for the slider
    connect(audioPlayer, &AudioPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(audioPlayer, &AudioPlayer::durationChanged, this, &MainWindow::updateDuration);
}

void MainWindow::playNextSong()
{
    if (!isPlaying) return;
    
    // Check if we have a pre-generated next song
    if (!nextSongFilePath.isEmpty()) {
        ui->statusLabel->setText("Playing: " + QFileInfo(nextSongFilePath).baseName());
        audioPlayer->play(nextSongFilePath);
        nextSongFilePath.clear();
        
        // Update current index to the next song
        int nextIndex = songModel->findNextIndex(currentSongIndex, shuffleMode);
        if (nextIndex >= 0 && nextIndex < songModel->rowCount()) {
            currentSongIndex = nextIndex;
        }
        
        // Start generating the song after this one
        startNextSongGeneration();
    } else {
        // Find next song index and generate it
        int nextIndex = songModel->findNextIndex(currentSongIndex, shuffleMode);
        
        if (nextIndex >= 0 && nextIndex < songModel->rowCount()) {
            currentSongIndex = nextIndex;
            generateAndPlayNext();
        } else {
            // No more songs
            isPlaying = false;
            isPaused = false;
            ui->statusLabel->setText("Finished playback");
            updateControls();
        }
    }
}

void MainWindow::generationError(const QString &error)
{
    // Reset the generation flag on error
    isGeneratingNext = false;
    
    // Show detailed error in a dialog with QTextEdit
    QDialog dialog(this);
    dialog.setWindowTitle("Generation Error");
    dialog.resize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *errorLabel = new QLabel("Error occurred during music generation:");
    errorLabel->setStyleSheet("font-weight: bold; color: darkred;");
    layout->addWidget(errorLabel);
    
    QTextEdit *errorTextEdit = new QTextEdit();
    errorTextEdit->setReadOnly(true);
    errorTextEdit->setPlainText(error);
    errorTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    errorTextEdit->setFontFamily("Monospace");
    layout->addWidget(errorTextEdit);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    
    dialog.exec();
    
    isPlaying = false;
    isPaused = false;
    updateControls();
}

void MainWindow::generationFinished()
{
    // Reset the generation flag when a generation completes
    isGeneratingNext = false;
}

void MainWindow::updatePlaybackStatus(bool playing)
{
    isPlaying = playing;
    updateControls();
}

void MainWindow::on_positionSlider_sliderMoved(int position)
{
    if (isPlaying && audioPlayer->isPlaying()) {
        // Seek to the new position when slider is moved
        audioPlayer->setPosition(position);
    }
}