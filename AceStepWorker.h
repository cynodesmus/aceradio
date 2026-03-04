#ifndef ACESTEPWORKER_H
#define ACESTEPWORKER_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <QString>
#include <QJsonObject>

class AceStepWorker : public QObject
{
    Q_OBJECT
public:
    explicit AceStepWorker(QObject *parent = nullptr);
    ~AceStepWorker();
    
    void generateSong(const QString &caption, const QString &lyrics, const QString &jsonTemplate,
                     const QString &aceStepPath, const QString &qwen3ModelPath,
                     const QString &textEncoderModelPath, const QString &ditModelPath,
                     const QString &vaeModelPath);
    void cancelGeneration();
    
signals:
    void songGenerated(const QString &filePath);
    void generationFinished();
    void generationError(const QString &error);
    void progressUpdate(int percent);
    
private slots:
    void workerFinished();
    
private:
    class Worker : public QRunnable {
    public:
        Worker(AceStepWorker *parent, const QString &caption, const QString &lyrics, const QString &jsonTemplate,
               const QString &aceStepPath, const QString &qwen3ModelPath,
               const QString &textEncoderModelPath, const QString &ditModelPath,
               const QString &vaeModelPath)
            : parent(parent), caption(caption), lyrics(lyrics), jsonTemplate(jsonTemplate),
              aceStepPath(aceStepPath), qwen3ModelPath(qwen3ModelPath),
              textEncoderModelPath(textEncoderModelPath), ditModelPath(ditModelPath),
              vaeModelPath(vaeModelPath) {}
        
        void run() override;
        
    private:
        AceStepWorker *parent;
        QString caption;
        QString lyrics;
        QString jsonTemplate;
        QString aceStepPath;
        QString qwen3ModelPath;
        QString textEncoderModelPath;
        QString ditModelPath;
        QString vaeModelPath;
    };
    
    Worker *currentWorker;
};

#endif // ACESTEPWORKER_H
