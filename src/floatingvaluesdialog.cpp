#include "floatingvaluesdialog.h"

FloatingValuesDialog::FloatingValuesDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    // set to sane default value;
    this->sourceWidget = 1;
    this->sourceChannel = 1;
    this->createUI();
}

void FloatingValuesDialog::setSourceWidget(const int &sourcew)
{
    this->sourceWidget = sourcew;
}

void FloatingValuesDialog::setSourceChannel(const int &channel)
{
    this->sourceChannel = channel;
}

void FloatingValuesDialog::setInputWidget(const int &w)
{
    this->stackedContainer->setCurrentIndex(w);

    // make sure the dialog is as small as possible
    this->resize(1, 1);
}

void FloatingValuesDialog::setInputWidgetValue(const double &value)
{
    // WARNING: This is highly dependend of our gui structure
    QFrame *cont =
        dynamic_cast<QFrame *>(this->stackedContainer->currentWidget());
    dynamic_cast<QDoubleSpinBox *>(cont->children()[1])->setValue(value);
}

void FloatingValuesDialog::setInputWidgetValue(const int &trackingMode) {}

void FloatingValuesDialog::updateDeviceSpecs(const double &voltageMin,
                                             const double &voltageMax,
                                             const uint &voltagePrecision,
                                             const double &currentMin,
                                             const double &currentMax,
                                             const uint &currentPrecision,
                                             const uint &noOfChannels)
{
    voltageSpinBox->setMinimum(voltageMin);
    voltageSpinBox->setMaximum(voltageMax);
    voltageSpinBox->setDecimals(voltagePrecision);

    currentSpinBox->setMinimum(currentMin);
    currentSpinBox->setMaximum(currentMax);
    currentSpinBox->setDecimals(currentPrecision);
}

void FloatingValuesDialog::createUI()
{
    this->mainLayout = new QGridLayout(this);
    this->setLayout(mainLayout);
    this->stackedContainer = new QStackedWidget();
    mainLayout->addWidget(this->stackedContainer, 0, 0);

    QFrame *frameVoltage = new QFrame();
    stackedContainer->addWidget(frameVoltage);
    QGridLayout *voltageLayout = new QGridLayout();
    frameVoltage->setLayout(voltageLayout);
    voltageLayout->setHorizontalSpacing(0);
    voltageLayout->setVerticalSpacing(0);
    voltageLayout->setMargin(0);
    this->voltageSpinBox = new QDoubleSpinBox();
    voltageSpinBox->setSuffix(" V");
    voltageSpinBox->setAlignment(Qt::AlignRight);
    voltageLayout->addWidget(voltageSpinBox);

    QFrame *frameCurrent = new QFrame();
    stackedContainer->addWidget(frameCurrent);
    QGridLayout *currentLayout = new QGridLayout();
    frameCurrent->setLayout(currentLayout);
    currentLayout->setHorizontalSpacing(0);
    currentLayout->setVerticalSpacing(0);
    currentLayout->setMargin(0);
    this->currentSpinBox = new QDoubleSpinBox();
    currentSpinBox->setSuffix(" A");
    currentSpinBox->setAlignment(Qt::AlignRight);
    currentLayout->addWidget(currentSpinBox);

    // TODO emkplement the other stacked Widgets

    this->acceptButton = new QToolButton();
    acceptButton->setText("");
    QIcon acceptIcon;
    acceptIcon.addPixmap(QPixmap(":/icons/dialog_accept.png"));
    acceptButton->setIcon(acceptIcon);

    this->mainLayout->addWidget(this->acceptButton, 0, 1);

    QObject::connect(this->acceptButton, SIGNAL(clicked()), this,
                     SLOT(accept()));
}

void FloatingValuesDialog::accept()
{
    qDebug() << Q_FUNC_INFO << "Dialog was accepted";
    QFrame *cont =
        dynamic_cast<QFrame *>(this->stackedContainer->currentWidget());
    double value = dynamic_cast<QDoubleSpinBox *>(cont->children()[1])->value();
    emit this->doubleValueAccepted(value, this->sourceWidget,
                                   this->sourceChannel);
    this->done(1);
}

void FloatingValuesDialog::reject()
{
    qDebug() << Q_FUNC_INFO << "Dialog was rejected";
    this->done(0);
}
