#pragma once

#include <KRunner/AbstractRunner>

#include <QRegularExpression>

class CsvRunner final : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    explicit CsvRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);

    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

private:
    struct CsvEntry {
        QString key;
        QString value;
        QString sourceFile;
    };

    [[nodiscard]] QVector<CsvEntry> readAllEntries() const;
    [[nodiscard]] QStringList csvDirectories() const;
    [[nodiscard]] QString readSecretFromPass(const QString &entryName) const;
    [[nodiscard]] QString readOtpFromPass(const QString &entryName) const;
    static bool isMailAddress(const QString &value);
    static QString iconNameForValue(const QString &value);
    static void copyToClipboard(const QString &text);

    QRegularExpression m_separatorRegex;
};