#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QMessageBox>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QDataStream>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ClientWindow), socket(new QTcpSocket(this)), isRecording(false), inCall(false), audioInput(nullptr), audioOutput(nullptr)
{
    ui->setupUi(this);

    // Setup UI connections
    connect(ui->connectButton, &QPushButton::clicked, this, &ClientWindow::connectToServer);
    connect(ui->disconnectButton, &QPushButton::clicked, this, &ClientWindow::disconnectFromServer);
    connect(ui->sendButton, &QPushButton::clicked, this, &ClientWindow::sendTextMessage);
    connect(ui->recordButton, &QPushButton::pressed, this, &ClientWindow::startRecording);
    connect(ui->recordButton, &QPushButton::released, this, &ClientWindow::stopRecording);
    connect(ui->callButton, &QPushButton::clicked, this, &ClientWindow::startCall);
    connect(ui->endCallButton, &QPushButton::clicked, this, &ClientWindow::endCall);

    // Setup socket connections
    connect(socket, &QTcpSocket::connected, this, &ClientWindow::onSocketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ClientWindow::onSocketDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, [this]()
            {
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_6_0);

        quint32 blockSize = 0;
        if (socket->bytesAvailable() < sizeof(quint32))
            return;
        in >> blockSize;

        if (socket->bytesAvailable() < blockSize)
            return;

        quint8 type;
        in >> type;

        switch (type) {
        case 1: { // Text message
            QString sender, message;
            in >> sender >> message;
            onTextMessageReceived(sender, message);
            break;
        }
        case 2: { // Voice message
            QString sender;
            QByteArray audioData;
            in >> sender >> audioData;
            onVoiceMessageReceived(sender, audioData);
            break;
        }
        case 3: { // Call request
            QString caller;
            in >> caller;
            onCallRequestReceived(caller);
            break;
        }
        case 4: { // Call response
            bool accepted;
            QString callee;
            in >> accepted >> callee;
            onCallResponseReceived(accepted, callee);
            break;
        }
        case 5: { // Call ended
            QString peer;
            in >> peer;
            onCallEnded(peer);
            break;
        }
        case 6: { // Voice data
            QByteArray data;
            in >> data;
            onVoiceDataReceived(data);
            break;
        }
        default:
            qDebug() << "Unknown message type";
        } });

    setupAudio();
    ui->disconnectButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->recordButton->setEnabled(false);
    ui->callButton->setEnabled(false);
    ui->endCallButton->setEnabled(false);
}

ClientWindow::~ClientWindow()
{
    delete ui;
    if (audioInput)
    {
        audioInput->stop();
        delete audioInput;
    }
    if (audioOutput)
    {
        audioOutput->stop();
        delete audioOutput;
    }
}

void ClientWindow::setupAudio()
{
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
}

void ClientWindow::connectToServer()
{
    userName = ui->nameEdit->text().trimmed();
    if (userName.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please enter your name");
        return;
    }

    QString serverAddress = ui->serverEdit->text().trimmed();
    quint16 port = ui->portEdit->text().toUShort();

    socket->connectToHost(serverAddress, port);
}

void ClientWindow::disconnectFromServer()
{
    endCall();
    socket->disconnectFromHost();
}

void ClientWindow::onSocketConnected()
{
    ui->connectButton->setEnabled(false);
    ui->disconnectButton->setEnabled(true);
    ui->sendButton->setEnabled(true);
    ui->recordButton->setEnabled(true);
    ui->callButton->setEnabled(true);

    // Send user name to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(0) << userName;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    socket->write(block);

    appendMessage("System", "Connected to server", false);
}

void ClientWindow::onSocketDisconnected()
{
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->recordButton->setEnabled(false);
    ui->callButton->setEnabled(false);
    ui->endCallButton->setEnabled(false);

    appendMessage("System", "Disconnected from server", false);
}

void ClientWindow::sendTextMessage()
{
    QString message = ui->messageEdit->toPlainText().trimmed();
    if (message.isEmpty())
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(1) << message;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    socket->write(block);

    appendMessage(userName, message, true);
    ui->messageEdit->clear();
}

void ClientWindow::startRecording()
{
    if (isRecording)
        return;

    isRecording = true;
    inputBuffer.open(QIODevice::WriteOnly);
    inputBuffer.buffer().clear();

    audioInput = new QAudioInput(format, this);
    connect(audioInput, &QAudioInput::stateChanged, this, &ClientWindow::handleAudioInputStateChanged);
    audioInput->start(&inputBuffer);

    ui->recordButton->setText("Recording...");
}

void ClientWindow::stopRecording()
{
    if (!isRecording)
        return;

    isRecording = false;
    audioInput->stop();
    delete audioInput;
    audioInput = nullptr;
    inputBuffer.close();

    QByteArray audioData = inputBuffer.buffer();
    if (!audioData.isEmpty())
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint32(0) << quint8(2) << audioData;
        out.device()->seek(0);
        out << quint32(block.size() - sizeof(quint32));
        socket->write(block);

        appendMessage(userName, "[Voice message]", true);
    }

    ui->recordButton->setText("Hold to Record");
}

void ClientWindow::startCall()
{
    QString callee = ui->callEdit->text().trimmed();
    if (callee.isEmpty() || callee == userName)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(3) << callee;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    socket->write(block);

    inCall = true;
    callPeer = callee;
    ui->callButton->setEnabled(false);
    ui->endCallButton->setEnabled(true);

    // Start audio streaming
    outputBuffer.open(QIODevice::ReadWrite);
    audioOutput = new QAudioOutput(format, this);
    connect(audioOutput, &QAudioOutput::stateChanged, this, &ClientWindow::handleAudioOutputStateChanged);
    audioOutput->start(&outputBuffer);

    inputBuffer.open(QIODevice::WriteOnly);
    audioInput = new QAudioInput(format, this);
    connect(audioInput, &QAudioInput::stateChanged, this, &ClientWindow::handleAudioInputStateChanged);
    connect(audioInput, &QAudioInput::notify, this, [this]()
            {
        if (inputBuffer.pos() > 1024) {
            QByteArray data = inputBuffer.buffer();
            inputBuffer.buffer().clear();
            inputBuffer.seek(0);

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_6_0);
            out << quint32(0) << quint8(6) << data;
            out.device()->seek(0);
            out << quint32(block.size() - sizeof(quint32));
            socket->write(block);
        } });
    audioInput->setNotifyInterval(100);
    audioInput->start(&inputBuffer);

    appendMessage("System", QString("Call started with %1").arg(callee), false);
}

void ClientWindow::endCall()
{
    if (!inCall)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(5) << callPeer;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    socket->write(block);

    if (audioInput)
    {
        audioInput->stop();
        delete audioInput;
        audioInput = nullptr;
    }
    if (audioOutput)
    {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = nullptr;
    }
    inputBuffer.close();
    outputBuffer.close();

    inCall = false;
    ui->callButton->setEnabled(true);
    ui->endCallButton->setEnabled(false);

    appendMessage("System", QString("Call ended with %1").arg(callPeer), false);
    callPeer.clear();
}

void ClientWindow::onTextMessageReceived(const QString &sender, const QString &message)
{
    appendMessage(sender, message, false);
}

void ClientWindow::onVoiceMessageReceived(const QString &sender, const QByteArray &audioData)
{
    appendMessage(sender, "[Voice message]", false);
    playAudio(audioData);
}

void ClientWindow::onCallRequestReceived(const QString &caller)
{
    if (inCall)
    {
        // Reject call if already in another call
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint32(0) << quint8(4) << false << caller;
        out.device()->seek(0);
        out << quint32(block.size() - sizeof(quint32));
        socket->write(block);
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Incoming Call",
                                  QString("%1 is calling. Accept?").arg(caller),
                                  QMessageBox::Yes | QMessageBox::No);

    bool accepted = (reply == QMessageBox::Yes);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(4) << accepted << caller;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    socket->write(block);

    if (accepted)
    {
        inCall = true;
        callPeer = caller;
        ui->callButton->setEnabled(false);
        ui->endCallButton->setEnabled(true);

        // Start audio streaming
        outputBuffer.open(QIODevice::ReadWrite);
        audioOutput = new QAudioOutput(format, this);
        connect(audioOutput, &QAudioOutput::stateChanged, this, &ClientWindow::handleAudioOutputStateChanged);
        audioOutput->start(&outputBuffer);

        inputBuffer.open(QIODevice::WriteOnly);
        audioInput = new QAudioInput(format, this);
        connect(audioInput, &QAudioInput::stateChanged, this, &ClientWindow::handleAudioInputStateChanged);
        connect(audioInput, &QAudioInput::notify, this, [this]()
                {
            if (inputBuffer.pos() > 1024) {
                QByteArray data = inputBuffer.buffer();
                inputBuffer.buffer().clear();
                inputBuffer.seek(0);

                QByteArray block;
                QDataStream out(&block, QIODevice::WriteOnly);
                out.setVersion(QDataStream::Qt_6_0);
                out << quint32(0) << quint8(6) << data;
                out.device()->seek(0);
                out << quint32(block.size() - sizeof(quint32));
                socket->write(block);
            } });
        audioInput->setNotifyInterval(100);
        audioInput->start(&inputBuffer);

        appendMessage("System", QString("Call started with %1").arg(caller), false);
    }
}

void ClientWindow::onCallResponseReceived(bool accepted, const QString &callee)
{
    if (!accepted)
    {
        QMessageBox::information(this, "Call Rejected",
                                 QString("%1 rejected your call").arg(callee));
        return;
    }

    // Call was accepted, audio streams are already set up in startCall()
    appendMessage("System", QString("%1 accepted your call").arg(callee), false);
}

void ClientWindow::onCallEnded(const QString &peer)
{
    if (inCall && callPeer == peer)
    {
        endCall();
        appendMessage("System", QString("%1 ended the call").arg(peer), false);
    }
}

void ClientWindow::onVoiceDataReceived(const QByteArray &data)
{
    if (inCall)
    {
        outputBuffer.seek(outputBuffer.size());
        outputBuffer.write(data);
    }
}

void ClientWindow::handleAudioInputStateChanged(QAudio::State state)
{
    if (state == QAudio::StoppedState && audioInput)
    {
        if (audioInput->error() != QAudio::NoError)
        {
            qDebug() << "Audio input error:" << audioInput->error();
        }
    }
}

void ClientWindow::handleAudioOutputStateChanged(QAudio::State state)
{
    if (state == QAudio::StoppedState && audioOutput)
    {
        if (audioOutput->error() != QAudio::NoError)
        {
            qDebug() << "Audio output error:" << audioOutput->error();
        }
    }
}

void ClientWindow::playAudio(const QByteArray &audioData)
{
    QBuffer *buffer = new QBuffer(this);
    buffer->setData(audioData);
    buffer->open(QIODevice::ReadOnly);

    QAudioOutput *audio = new QAudioOutput(format, this);
    connect(audio, &QAudioOutput::stateChanged, this, [audio, buffer](QAudio::State state)
            {
        if (state == QAudio::StoppedState) {
            buffer->close();
            buffer->deleteLater();
            audio->deleteLater();
        } });
    audio->start(buffer);
}

void ClientWindow::appendMessage(const QString &sender, const QString &message, bool isMyMessage)
{
    QString formattedMessage;
    if (isMyMessage)
    {
        formattedMessage = QString("<div align='right'><b>%1:</b> %2</div>")
                               .arg(sender, message);
    }
    else
    {
        formattedMessage = QString("<div align='left'><b>%1:</b> %2</div>")
                               .arg(sender, message);
    }

    ui->messagesView->append(formattedMessage);
}