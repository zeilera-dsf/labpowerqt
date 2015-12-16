#include "powersupplyscpi.h"

namespace powcon = PowerSupplySCPI_constants;

PowerSupplySCPI::PowerSupplySCPI(QString serialPortName, QString deviceName,
                                 int noOfChannels, int voltageAccuracy,
                                 int currentAccuracy, QObject *parent)
    : serialPortName(std::move(serialPortName)),
      deviceName(std::move(deviceName)), noOfChannels(noOfChannels),
      voltageAccuracy(voltageAccuracy), currentAccuracy(currentAccuracy),
      QObject(parent)
{
    this->serialPort = nullptr;
    this->canCalculateWattage = false;
}

PowerSupplySCPI::~PowerSupplySCPI() {}

void PowerSupplySCPI::startPowerSupplyBackgroundThread()
{
    this->backgroundWorkerThreadRun = true;
    this->threadFunc();
}

void PowerSupplySCPI::stopPowerSupplyBackgroundThread()
{
    this->backgroundWorkerThreadRun = false;
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::SETDUMMY));
}

QString PowerSupplySCPI::getserialPortName() { return this->serialPortName; }

QString PowerSupplySCPI::getDeviceName() { return this->deviceName; }

void PowerSupplySCPI::threadFunc()
{
    this->serialPort = new QSerialPort(this->serialPortName);

    // this->serialPort = new QSerialPort(this->serialPortName);
    if (!this->serialPort->open(QIODevice::ReadWrite)) {
        emit errorOpen(this->serialPort->errorString());
        return;
    }

    this->serialPort->setBaudRate(this->port_baudraute);
    this->serialPort->setFlowControl(this->port_flowControl);
    this->serialPort->setDataBits(this->port_databits);
    this->serialPort->setParity(this->port_parity);
    this->serialPort->setStopBits(this->port_stopbits);

    emit deviceOpen();

    while (this->backgroundWorkerThreadRun) {
        this->readWriteData(this->serQueue.pop());
    }

    qDebug() << Q_FUNC_INFO << "Background thread stopped";

    QMutexLocker qlock(&this->qserialPortGuard);
    if (this->serialPort && this->serialPort->isOpen()) {
        this->serialPort->close();
        delete this->serialPort;
    }

    emit backgroundThreadStopped();
}

void PowerSupplySCPI::readWriteData(std::shared_ptr<SerialCommand> com)
{
    // FIXME: There is a race condition if the destructor of a derived class is
    // called because we use several pure virtual methods here.
    // Solutions: 1. Provide basic implementations in base class :(
    // 2. Wait in derived destructor until serialPortGuard is available. This
    // sounds good but every derived class needs to do this. Not convenient and
    // most of all not error prone.

    QMutexLocker qlock(&this->qserialPortGuard);
    std::shared_ptr<PowerSupplyStatus> status = nullptr;
    std::vector<std::shared_ptr<SerialCommand>> commands = {com};

    if (com->getCommand() == powcon::COMMANDS::SETDUMMY) {
        return;
    }

    std::chrono::high_resolution_clock::time_point tStart =
        std::chrono::high_resolution_clock::now();

    if (com->getCommand() == powcon::GETSTATUS) {
        commands = this->prepareStatusCommands();
        status = std::make_shared<PowerSupplyStatus>();
    }

    // TODO: The Timeout values for serial port operations are critical. Maybe
    // they need to be configurable
    for (auto &c : commands) {
        QByteArray commandByte = this->prepareCommand(c);
        bool waitForBytes = true;
        if (this->serialPort->write(commandByte) != -1) {
            // wait for for bytes to be written
            if (commandByte != "")
                waitForBytes = this->serialPort->waitForBytesWritten(1000);
        }

        if (waitForBytes) {
            // is this command with a feedback?
            if (c->getCommandWithReply()) {
                QByteArray reply;
                if (commandByte != "") {
                    // wait until port is ready to read
                    this->serialPort->waitForReadyRead(1000);
                    reply = this->serialPort->readAll();
                    while (this->serialPort->waitForReadyRead(10)) {
                        reply += this->serialPort->readAll();
                    }
                }
                c->setValue(QVariant(reply));

                if (status != nullptr) {
                    this->processStatusCommands(status, c);
                } else {
                    // send our com object with the signal
                    emit this->requestFinished(c);
                }
            }
        } else {
            qDebug() << Q_FUNC_INFO
                     << "Could not write to serial port. Error number: "
                     << this->serialPort->error();
            ;
            // TODO emit an error
        }
    }

    std::chrono::high_resolution_clock::time_point tEnd =
        std::chrono::high_resolution_clock::now();
    long duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart)
            .count();
    qDebug() << Q_FUNC_INFO << "Elapsed time for serial command(s): " << duration
             << "ms";

    if (status != nullptr) {
        status->setTime(std::move(std::chrono::system_clock::now()));
        // calculate wattage
        if (this->canCalculateWattage)
            this->calculateWattage(status);
        emit this->statusReady(status);
    }
}

std::vector<std::shared_ptr<SerialCommand>>
PowerSupplySCPI::prepareStatusCommands()
{
    std::vector<std::shared_ptr<SerialCommand>> comVec;
    for (const auto &c : this->statusCommands) {
        if (c == powcon::COMMANDS::GETACTUALVOLTAGE ||
            c == powcon::COMMANDS::GETACTUALCURRENT ||
            c == powcon::COMMANDS::GETVOLTAGE || powcon::COMMANDS::GETCURRENT) {
            for (int i = 1; i <= this->noOfChannels; i++) {
                std::shared_ptr<SerialCommand> com =
                    std::make_shared<SerialCommand>(static_cast<int>(c), i,
                                                    QVariant(), true);
                comVec.push_back(com);
            }
        } else {
            std::shared_ptr<SerialCommand> com = std::make_shared<SerialCommand>(
                static_cast<int>(c), 1, QVariant(), true);
            comVec.push_back(com);
        }
    }
    return comVec;
}
