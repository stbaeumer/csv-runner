#include "csvrunner.h"

#include <algorithm>

#include <KLocalizedString>
#include <KPluginFactory>

#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QTextStream>
#include <QUrl>

K_PLUGIN_CLASS_WITH_JSON(CsvRunner, "metadata.json")

CsvRunner::CsvRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KRunner::AbstractRunner(parent, metaData)
    , m_separatorRegex(QStringLiteral("\\s*\\|\\s*"))
{
    Q_UNUSED(args)
    setObjectName(QStringLiteral("csv-runner"));
    addSyntax(KRunner::RunnerSyntax(QStringLiteral(":q:"), i18n("Sucht Schlüssel aus CSV-Dateien und führt Aktionen auf den Wert aus")));
}

void CsvRunner::match(KRunner::RunnerContext &context)
{
    const QString query = context.query().trimmed();
    if (query.isEmpty()) {
        return;
    }

    const auto entries = readAllEntries();
    for (const auto &entry : entries) {
        if (!entry.key.contains(query, Qt::CaseInsensitive)) {
            continue;
        }

        KRunner::QueryMatch result(this);
        result.setId(entry.key + QLatin1Char('|') + entry.value + QLatin1Char('|') + entry.sourceFile);
        result.setRelevance(0.9);
        result.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Moderate);
        result.setText(entry.key);
        result.setSubtext(i18n("%1 (%2)", entry.value, entry.sourceFile));
        result.setIconName(iconNameForValue(entry.value));
        result.setData(QVariantMap{
            {QStringLiteral("key"), entry.key},
            {QStringLiteral("value"), entry.value},
        });

        context.addMatch(result);
    }
}

void CsvRunner::run(const KRunner::RunnerContext &, const KRunner::QueryMatch &match)
{
    const auto data = match.data().toMap();
    const QString key = data.value(QStringLiteral("key")).toString().trimmed();
    const QString value = data.value(QStringLiteral("value")).toString().trimmed();

    if (value.startsWith(QStringLiteral("https://"), Qt::CaseInsensitive)) {
        QDesktopServices::openUrl(QUrl(value));
        return;
    }

    if (value.startsWith(QStringLiteral("joplin://"), Qt::CaseInsensitive)) {
        QDesktopServices::openUrl(QUrl(value));
        return;
    }

    if (isMailAddress(value)) {
        QUrl mailUrl(QStringLiteral("mailto:%1").arg(value));
        QDesktopServices::openUrl(mailUrl);
        return;
    }

    if (value.compare(QStringLiteral("pass"), Qt::CaseInsensitive) == 0) {
        const QString password = readSecretFromPass(key);
        if (!password.isEmpty()) {
            copyToClipboard(password);
        }
        return;
    }

    if (value.compare(QStringLiteral("otp"), Qt::CaseInsensitive) == 0) {
        const QString otpCode = readOtpFromPass(key);
        if (!otpCode.isEmpty()) {
            copyToClipboard(otpCode);
        }
        return;
    }

    copyToClipboard(value);
}

QVector<CsvRunner::CsvEntry> CsvRunner::readAllEntries() const
{
    QVector<CsvEntry> entries;

    const QDir directory(csvDirectory());
    if (!directory.exists()) {
        return entries;
    }

    QStringList csvFiles;
    QDirIterator it(directory.absolutePath(), {QStringLiteral("*.csv")}, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        csvFiles.push_back(it.next());
    }
    std::sort(csvFiles.begin(), csvFiles.end(), [](const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });

    for (const QString &csvFilePath : csvFiles) {
        const QFileInfo fileInfo(csvFilePath);
        QFile file(csvFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            const QString line = stream.readLine().trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
                continue;
            }

            const QStringList parts = line.split(m_separatorRegex, Qt::KeepEmptyParts);
            if (parts.size() < 2) {
                continue;
            }

            CsvEntry entry;
            entry.key = parts.at(0).trimmed();
            entry.value = parts.mid(1).join(QStringLiteral("|")).trimmed();
            entry.sourceFile = directory.relativeFilePath(fileInfo.absoluteFilePath());

            if (!entry.key.isEmpty() && !entry.value.isEmpty()) {
                entries.push_back(entry);
            }
        }
    }

    return entries;
}

QString CsvRunner::csvDirectory() const
{
    const QString configured = qEnvironmentVariable("CSV_RUNNER_DIR").trimmed();
    if (!configured.isEmpty()) {
        return configured;
    }

    const QString preferred = QDir::homePath() + QStringLiteral("/.local/share/csv-runner");
    if (QDir(preferred).exists()) {
        return preferred;
    }

    return QDir::homePath() + QStringLiteral("/csv-runner");
}

QString CsvRunner::readSecretFromPass(const QString &entryName) const
{
    QProcess process;
    process.start(QStringLiteral("pass"), {entryName});
    if (!process.waitForFinished(6000) || process.exitCode() != 0) {
        return {};
    }

    const QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return {};
    }

    return lines.constFirst().trimmed();
}

QString CsvRunner::readOtpFromPass(const QString &entryName) const
{
    QProcess process;
    process.start(QStringLiteral("pass"), {QStringLiteral("otp"), entryName});
    if (!process.waitForFinished(6000) || process.exitCode() != 0) {
        return {};
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

bool CsvRunner::isMailAddress(const QString &value)
{
    static const QRegularExpression mailRegex(
        QStringLiteral(R"((^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$))"),
        QRegularExpression::CaseInsensitiveOption);
    return mailRegex.match(value.trimmed()).hasMatch();
}

QString CsvRunner::iconNameForValue(const QString &value)
{
    const QString trimmedValue = value.trimmed();

    if (trimmedValue.startsWith(QStringLiteral("https://"), Qt::CaseInsensitive)) {
        return QStringLiteral("www");
    }

    if (trimmedValue.startsWith(QStringLiteral("joplin://"), Qt::CaseInsensitive)) {
        return QStringLiteral("joplin");
    }

    if (isMailAddress(trimmedValue)) {
        return QStringLiteral("mail");
    }

    if (trimmedValue.compare(QStringLiteral("pass"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("pass");
    }

    if (trimmedValue.compare(QStringLiteral("otp"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("otp");
    }

    return QStringLiteral("csv-runner");
}

void CsvRunner::copyToClipboard(const QString &text)
{
    auto *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        return;
    }

    clipboard->setText(text, QClipboard::Clipboard);
}

#include "csvrunner.moc"