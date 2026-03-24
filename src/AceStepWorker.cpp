// Copyright Carl Philipp Klemm, cynodesmus 2026
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AceStepWorker.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRandomGenerator>
#include <QStandardPaths>

AceStep::AceStep(QObject * parent) : QObject(parent) {}

bool AceStep::isGenerating(SongItem * song) {
    if (busy && song) {
        *song = this->request.song;
    }
    return busy;
}

void AceStep::cancelGeneration() {
    if (qwenProcess) {
        qwenProcess->blockSignals(true);
        qwenProcess->kill();
        qwenProcess->waitForFinished();
        qwenProcess->blockSignals(false);
    }

    if (ditVaeProcess) {
        ditVaeProcess->blockSignals(true);
        ditVaeProcess->kill();
        ditVaeProcess->waitForFinished();
        ditVaeProcess->blockSignals(false);
    }

    progressUpdate(100);
    if (busy) {
        generationCanceled(request.song);
    }

    busy = false;
}

bool AceStep::requestGeneration(SongItem song,
                                QString  requestTemplate,
                                QString  aceStepPath,
                                QString  qwen3ModelPath,
                                QString  textEncoderModelPath,
                                QString  ditModelPath,
                                QString  vaeModelPath) {
    if (busy) {
        qWarning() << "Dropping song:" << song.caption;
        return false;
    }
    busy = true;

    if (!qwenProcess) {
        qwenProcess = new QProcess(this);
        connect(qwenProcess, &QProcess::finished, this, &AceStep::qwenProcFinished);
    }
    if (!ditVaeProcess) {
        ditVaeProcess = new QProcess(this);
        connect(ditVaeProcess, &QProcess::finished, this, &AceStep::ditProcFinished);
    }

    // Generate a unique ID based on time (YYYYMMDD_HHMMSS_ZZZ)
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    // Use the selected folder or default Temp
    QString baseDir =
        workingDir.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::TempLocation) : workingDir;

    request = { song, timestamp, 0, aceStepPath, textEncoderModelPath, ditModelPath, vaeModelPath };

    QString   qwen3Binary = aceStepPath + "/ace-lm" + EXE_EXT;
    QFileInfo qwen3Info(qwen3Binary);
    if (!qwen3Info.exists() || !qwen3Info.isExecutable()) {
        generationError("ace-lm binary not found at: " + qwen3Binary);
        busy = false;
        return false;
    }
    if (!QFileInfo::exists(qwen3ModelPath)) {
        generationError("Qwen3 model not found: " + qwen3ModelPath);
        busy = false;
        return false;
    }
    if (!QFileInfo::exists(textEncoderModelPath)) {
        generationError("Text encoder model not found: " + textEncoderModelPath);
        busy = false;
        return false;
    }
    if (!QFileInfo::exists(ditModelPath)) {
        generationError("DiT model not found: " + ditModelPath);
        busy = false;
        return false;
    }
    if (!QFileInfo::exists(vaeModelPath)) {
        generationError("VAE model not found: " + vaeModelPath);
        busy = false;
        return false;
    }

    request.requestFilePath = baseDir + "/req_" + timestamp + ".json";

    QJsonParseError parseError;
    QJsonDocument   templateDoc = QJsonDocument::fromJson(requestTemplate.toUtf8(), &parseError);
    if (!templateDoc.isObject()) {
        generationError("Invalid JSON template: " + QString(parseError.errorString()));
        busy = false;
        return false;
    }

    QJsonObject requestObj = templateDoc.object();
    song.store(requestObj);

    // Write the request file
    QFile requestFileHandle(request.requestFilePath);
    if (!requestFileHandle.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit generationError("Failed to create request file: " + requestFileHandle.errorString());
        busy = false;
        return false;
    }
    requestFileHandle.write(QJsonDocument(requestObj).toJson(QJsonDocument::Indented));
    requestFileHandle.close();

    QStringList qwen3Args;
    qwen3Args << "--request" << request.requestFilePath;
    qwen3Args << "--lm" << qwen3ModelPath;

    progressUpdate(30);

    qwenProcess->start(qwen3Binary, qwen3Args);
    return true;
}

void AceStep::qwenProcFinished(int code, QProcess::ExitStatus status) {
    // QFile::remove(request.requestFilePath); // COMMENTED: Save request history
    if (code != 0) {
        QString errorOutput = qwenProcess->readAllStandardError();
        generationError("ace-lm exited with code " + QString::number(code) + ": " + errorOutput);
        busy = false;
        return;
    }

    QString   ditVaeBinary = request.aceStepPath + "/ace-synth" + EXE_EXT;
    QFileInfo ditVaeInfo(ditVaeBinary);
    if (!ditVaeInfo.exists() || !ditVaeInfo.isExecutable()) {
        generationError("ace-synth binary not found at: " + ditVaeBinary);
        busy = false;
        return;
    }

    // Use the paths generated at the beginning of generation
    QString baseDir =
        workingDir.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::TempLocation) : workingDir;
    request.requestLlmFilePath = baseDir + "/req_" + request.uidStr + "0.json";
    if (!QFileInfo::exists(request.requestLlmFilePath)) {
        generationError("ace-lm failed to create enhanced request file " + request.requestLlmFilePath);
        busy = false;
        return;
    }

    // Load lyrics from the enhanced request file
    QFile lmOutputFile(request.requestLlmFilePath);
    if (lmOutputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonParseError parseError;
        request.song.json = lmOutputFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(request.song.json.toUtf8(), &parseError);
        lmOutputFile.close();

        if (doc.isObject() && !parseError.error) {
            QJsonObject obj = doc.object();
            if (obj.contains("lyrics") && obj["lyrics"].isString()) {
                request.song.lyrics = obj["lyrics"].toString();
            }
        }
    }

    // Step 2: Run ace-synth to generate audio
    QStringList ditVaeArgs;
    ditVaeArgs << "--request" << request.requestLlmFilePath;
    ditVaeArgs << "--embedding" << request.textEncoderModelPath;
    ditVaeArgs << "--dit" << request.ditModelPath;
    ditVaeArgs << "--vae" << request.vaeModelPath;
    ditVaeArgs << "--wav";

    progressUpdate(60);

    ditVaeProcess->start(ditVaeBinary, ditVaeArgs);
}

void AceStep::ditProcFinished(int code, QProcess::ExitStatus status) {
    // QFile::remove(request.requestLlmFilePath); // COMMENTED: Saving improved JSON
    if (code != 0) {
        QString errorOutput = ditVaeProcess->readAllStandardError();
        generationError("ace-synth exited with code " + QString::number(code) + ": " + errorOutput);
        busy = false;
        return;
    }

    // Find the generated WAV file
    QString baseDir =
        workingDir.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::TempLocation) : workingDir;
    QString wavFile = baseDir + "/req_" + request.uidStr + "00.wav";
    if (!QFileInfo::exists(wavFile)) {
        generationError("No WAV file generated at " + wavFile);
        busy = false;
        return;
    }
    busy = false;

    progressUpdate(100);
    request.song.file         = wavFile;
    request.song.originalJson = request.requestFilePath;
    request.song.enhancedJson = request.requestLlmFilePath;

    emit songGenerated(request.song);
}
