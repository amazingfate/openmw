﻿#include "settingspage.hpp"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include <components/files/qtconversion.hpp>

#include "utils/textinputdialog.hpp"

using namespace Process;

Launcher::SettingsPage::SettingsPage(Files::ConfigurationManager& cfg, Config::GameSettings& gameSettings,
    Config::LauncherSettings& launcherSettings, MainDialog* parent)
    : QWidget(parent)
    , mCfgMgr(cfg)
    , mGameSettings(gameSettings)
    , mLauncherSettings(launcherSettings)
    , mMain(parent)
{
    setupUi(this);

    QStringList languages;
    languages << tr("English") << tr("French") << tr("German") << tr("Italian") << tr("Polish") << tr("Russian")
              << tr("Spanish") << tr("Chinese(GBK)") << tr("UTF-8");

    languageComboBox->addItems(languages);

    mWizardInvoker = new ProcessInvoker();
    mImporterInvoker = new ProcessInvoker();
    resetProgressBar();

    connect(mWizardInvoker->getProcess(), &QProcess::started, this, &SettingsPage::wizardStarted);

    connect(mWizardInvoker->getProcess(), qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
        &SettingsPage::wizardFinished);

    connect(mImporterInvoker->getProcess(), &QProcess::started, this, &SettingsPage::importerStarted);

    connect(mImporterInvoker->getProcess(), qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
        &SettingsPage::importerFinished);

    mProfileDialog = new TextInputDialog(tr("New Content List"), tr("Content List name:"), this);

    connect(mProfileDialog->lineEdit(), &LineEdit::textChanged, this, &SettingsPage::updateOkButton);

    // Detect Morrowind configuration files
    QStringList iniPaths;

    for (const QString& path : mGameSettings.getDataDirs())
    {
        QDir dir(path);
        dir.setPath(dir.canonicalPath()); // Resolve symlinks

        if (dir.exists(QString("Morrowind.ini")))
            iniPaths.append(dir.absoluteFilePath(QString("Morrowind.ini")));
        else
        {
            if (!dir.cdUp())
                continue; // Cannot move from Data Files

            if (dir.exists(QString("Morrowind.ini")))
                iniPaths.append(dir.absoluteFilePath(QString("Morrowind.ini")));
        }
    }

    if (!iniPaths.isEmpty())
    {
        settingsComboBox->addItems(iniPaths);
        importerButton->setEnabled(true);
    }
    else
    {
        importerButton->setEnabled(false);
    }

    loadSettings();
}

Launcher::SettingsPage::~SettingsPage()
{
    delete mWizardInvoker;
    delete mImporterInvoker;
}

void Launcher::SettingsPage::on_wizardButton_clicked()
{
    mMain->writeSettings();

    if (!mWizardInvoker->startProcess(QLatin1String("openmw-wizard"), false))
        return;
}

void Launcher::SettingsPage::on_importerButton_clicked()
{
    mMain->writeSettings();

    // Create the file if it doesn't already exist, else the importer will fail
    auto path = mCfgMgr.getUserConfigPath();
    path /= "openmw.cfg";
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QFile file(path);
#else
    QFile file(Files::pathToQString(path));
#endif

    if (!file.exists())
    {
        if (!file.open(QIODevice::ReadWrite))
        {
            // File cannot be created
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("写入 OpenMW 配置文件出错"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setText(
                tr("<html><head/><body><p><b>无法打开或创建 %1</b></p> \
                              <p>请确认你有相关权限\
                              并再次尝试。</p></body></html>")
                    .arg(file.fileName()));
            msgBox.exec();
            return;
        }

        file.close();
    }

    // Construct the arguments to run the importer
    QStringList arguments;

    if (addonsCheckBox->isChecked())
        arguments.append(QString("--game-files"));

    if (fontsCheckBox->isChecked())
        arguments.append(QString("--fonts"));

    arguments.append(QString("--encoding"));
    arguments.append(mGameSettings.value(QString("encoding"), QString("win1252")));
    arguments.append(QString("--ini"));
    arguments.append(settingsComboBox->currentText());
    arguments.append(QString("--cfg"));
    arguments.append(Files::pathToQString(path));

    qDebug() << "arguments " << arguments;

    // start the progress bar as a "bouncing ball"
    progressBar->setMaximum(0);
    progressBar->setValue(0);
    if (!mImporterInvoker->startProcess(QLatin1String("openmw-iniimporter"), arguments, false))
    {
        resetProgressBar();
    }
}

void Launcher::SettingsPage::on_browseButton_clicked()
{
    QString iniFile = QFileDialog::getOpenFileName(this, QObject::tr("选择配置文件"), QDir::currentPath(),
        QString(tr("晨风配置文件 (*.ini)")));

    if (iniFile.isEmpty())
        return;

    QFileInfo info(iniFile);

    if (!info.exists() || !info.isReadable())
        return;

    const QString path(QDir::toNativeSeparators(info.absoluteFilePath()));

    if (settingsComboBox->findText(path) == -1)
    {
        settingsComboBox->addItem(path);
        settingsComboBox->setCurrentIndex(settingsComboBox->findText(path));
        importerButton->setEnabled(true);
    }
}

void Launcher::SettingsPage::wizardStarted()
{
    mMain->hide(); // Hide the launcher

    wizardButton->setEnabled(false);
}

void Launcher::SettingsPage::wizardFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus == QProcess::CrashExit)
        return qApp->quit();

    mMain->reloadSettings();
    wizardButton->setEnabled(true);

    mMain->show(); // Show the launcher again
}

void Launcher::SettingsPage::importerStarted()
{
    importerButton->setEnabled(false);
}

void Launcher::SettingsPage::importerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus == QProcess::CrashExit)
    {
        resetProgressBar();

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("导入完成"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("从 INI 文件导入设置失败。"));
        msgBox.exec();
    }
    else
    {
        // indicate progress finished
        progressBar->setMaximum(1);
        progressBar->setValue(1);

        // Importer may have changed settings, so refresh
        mMain->reloadSettings();
    }

    importerButton->setEnabled(true);
}

void Launcher::SettingsPage::resetProgressBar()
{
    // set progress bar to 0 %
    progressBar->reset();
}

void Launcher::SettingsPage::updateOkButton(const QString& text)
{
    // We do this here because we need to access the profiles
    if (text.isEmpty())
    {
        mProfileDialog->setOkButtonEnabled(false);
        return;
    }

    const QStringList profiles(mLauncherSettings.getContentLists());

    (profiles.contains(text)) ? mProfileDialog->setOkButtonEnabled(false) : mProfileDialog->setOkButtonEnabled(true);
}

void Launcher::SettingsPage::saveSettings()
{
    QString language(languageComboBox->currentText());

    mLauncherSettings.setValue(QLatin1String("Settings/language"), language);

    if (language == QLatin1String("Polish"))
    {
        mGameSettings.setValue(QLatin1String("encoding"), QLatin1String("win1250"));
    }
    else if (language == QLatin1String("Russian"))
    {
        mGameSettings.setValue(QLatin1String("encoding"), QLatin1String("win1251"));
    }
    else if (language == QLatin1String("Chinese(GBK)"))
    {
        mGameSettings.setValue(QLatin1String("encoding"), QLatin1String("gbk"));
    }
    else if (language == QLatin1String("UTF-8"))
    {
        mGameSettings.setValue(QLatin1String("encoding"), QLatin1String("utf8"));
    }
    else
    {
        mGameSettings.setValue(QLatin1String("encoding"), QLatin1String("win1252"));
    }
}

bool Launcher::SettingsPage::loadSettings()
{
    QString language(mLauncherSettings.value(QLatin1String("Settings/language")));

    int index = languageComboBox->findText(language);

    if (index != -1)
        languageComboBox->setCurrentIndex(index);

    return true;
}
