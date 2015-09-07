// labpowerqt is a Gui application to control programmable lab power supplies
// Copyright © 2015 Christian Rapp <0x2a at posteo dot org>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QQuickView>
#include <QAbstractButton>
#include <QDialogButtonBox>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QDebug>
#include <QSettings>

#include <memory>

#include "settingsdefinitions.h"
#include "serialcommand.h"
#include "koradscpi.h"

namespace Ui
{
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;

    std::unique_ptr<PowerSupplySCPI> powerSupplyConnector;
    bool deviceConnected;

    void setupSettingsList();

private slots:
    void settingChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void buttonBoxClicked(QAbstractButton *button);
    void testClicked();

    void deviceIdentified(std::shared_ptr<SerialCommand> command);
    void deviceOpenError(QString errorString);

    void accept();
    void reject();
};

#endif // SETTINGSDIALOG_H
