#define WIN32_LEAN_AND_MEAN

#include "header.h"
#include <QDateTime>
#include <chrono>
using namespace std::chrono;

int result;

WSADATA wsaData;
ADDRINFO hints;
ADDRINFO *addrResult = NULL;
SOCKET ConnectSocket = INVALID_SOCKET;

std::string username;
std::string companionname = "Your interlocutor";
std::string key{"verysecretkey"};
std::string str;

std::string sendStr;
std::string sendStr2;
std::string sendSTR;
std::string recvStr;
std::string recvStr2;
std::string recvSTR;
std::string fileContent;

std::string markMES = "M:";
std::string markAUD = "A:";

std::vector<QString*> yqstr;
std::vector<QString*> oqstr;

QString message;
QString mes;
QString ymes;
QString opmes;
QString warning;

QString RECVQSTR;
QByteArray audioMessage;

std::thread T;
std::thread T2;
std::thread T3;

bool answ = false;

bool y = false;
bool o = false;

//maybe lishnee
bool RES;

// true - m false - audio

int maxmes = 20;
int position = 0;

QString inputFile = "MES.wav";

std::string fileToString(const QString& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning("Не удалось открыть файл!");
        return "";
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8); // Указываем кодировку (если нужно)
    QString content = in.readAll();
    file.close();

    return content.toStdString();
}

bool  windows::convertFromAscii(const QString &txtFilePath, const QString &wavFilePath)
{
    QFile txtFile(txtFilePath);

    if (!txtFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "TXT FILE ERROR:" << txtFile.errorString();
        return false;
    }

    QString hexString = QString::fromUtf8(txtFile.readAll());
    txtFile.close();

    // Удаляем все пробелы и переносы строк из строки
    hexString = hexString.simplified();
    hexString.remove(' ');

    // Проверяем, что длина строки четная (каждый байт представлен 2 символами)
    if (hexString.length() % 2 != 0)
    {
        qWarning() << "Invalid hex string length";
        return false;
    }

    QByteArray wavData;
    bool ok;

    // Конвертируем каждую пару символов обратно в байт
    for (int i = 0; i < hexString.length(); i += 2)
    {
        QString byteStr = hexString.mid(i, 2);
        quint8 byte = byteStr.toUShort(&ok, 16);

        if (!ok)
        {
            qWarning() << "Invalid hex data at position" << i;
            return false;
        }

        wavData.append(byte);
    }

    QFile wavFile(wavFilePath);

    if (!wavFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "WAV FILE ERROR:" << wavFile.errorString();
        return false;
    }

    wavFile.write(wavData);
    wavFile.close();

    return true;
}

bool saveStringToFileQt(const QString &filePath, const QString &content) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл для записи!";
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);  // Установка кодировки UTF-8
    out << content;
    file.close();

    return true;
}

void body :: coding(std::string& str)
{
    for(size_t i =0; i<str.size(); i++)
    {
        str[i]=str[i]^key[i%key.size()];
    }
    str = markMES + str;
}

void body :: decoding(std::string& str)
{
    str.erase(0, 2);
    for(size_t i =0; i<str.size(); i++)
    {
        str[i]=str[i]^key[i%key.size()];
    }
}

void body :: work()
{
    do
    {
        std::this_thread::sleep_for(100ms);
        if (answ == true)
        {
            int SIZE = (int)(sendStr.size());

            if(SIZE >= 1)
            {
                sendSTR = sendStr;
                coding(sendStr);
                sendStr2 = "You : " + sendSTR;
            }

            else if(SIZE == 0)
            {
                //fileContent = fileToString("output.txt");
                sendStr = markAUD;
                sendStr.append(audioMessage.data(), audioMessage.size());
            }

            y = true;

            result = send(ConnectSocket, sendStr.c_str(), (int)(sendStr.size()), 0);

            if (result == SOCKET_ERROR)
            {
                std::cout << "Mistake with sending message" << std::endl;
                working = false;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return;
            }

            sendSTR.clear();
            sendStr.clear();
            answ = false;
        }

    } while (working);
}

void body :: doing()
{
    std::thread T([&](){work();});

    const int MAX = 2000000;
    char recvBuffer[MAX];

    do
    {
        result = recv(ConnectSocket, recvBuffer, MAX, 0);

        if (result > 0)
        {

            recvStr.assign(recvBuffer,recvBuffer+result);
            recvSTR = recvStr;

            if(recvSTR[0] == 'M')
            {
                decoding(recvStr);
                recvStr2 = companionname + " : " + recvStr;
                recvSTR.erase(0, 2);
            }

            if(recvSTR[0] == 'A')
            {
                while(result==MAX){
                    result = recv(ConnectSocket, recvBuffer, MAX, 0);
                    recvStr.append(recvBuffer,result);
                }
                recvStr.erase(0, 2);
                recvStr2.clear();

                QFile wavFile("last_audio_message.wav");

                if (!wavFile.open(QIODevice::WriteOnly))
                {
                    qWarning() << "WAV FILE ERROR:" << wavFile.errorString();
                    return;
                }
                wavFile.write(recvStr.data(), recvStr.size());
                wavFile.close();
            }
            recvSTR.clear();
            recvStr.clear();
            o = true;
        }
        else
        {
            std::cout << "Mistake with connecting? Client will be disable, write anything" << std::endl;
            working = false;
            T.join();
            break;
        }
    } while (working);

}

void windows :: setupSettings()
{
    // Получаем устройство записи по умолчанию
    QAudioDevice device = QMediaDevices::defaultAudioInput();
    if (device.isNull())
    {
        qWarning() << "No audio input device found!";
        return;
    }

    // Настраиваем формат аудио
    QAudioFormat format;
    format.setSampleRate(44100); // Частота дискретизации 44.1 кГц
    format.setChannelCount(1);   // Моно
    format.setSampleFormat(QAudioFormat::Int16); // 16-битный формат

    // Проверяем поддержку формата
    if (!device.isFormatSupported(format))
    {
        qWarning() << "Default format not supported, using nearest.";
        format = device.preferredFormat();
    }

    // Создаем аудиовход
    audioInput = new QAudioInput(device);

    // Настраиваем сессию записи
    captureSession = new QMediaCaptureSession(this);
    captureSession->setAudioInput(audioInput);

    // Создаем рекордер
    recorder = new QMediaRecorder(this);
    captureSession->setRecorder(recorder);

    QMediaFormat WAV(QMediaFormat::Wave);
    // Устанавливаем формат записи (WAV)
    recorder->setMediaFormat(WAV);
}

void windows :: creating()
{
    window = new QWidget();
    window -> setWindowTitle("CLIENT");

    Sending = new QPushButton("Send", window);

    Texting = new QLineEdit(message, window);

    startButton = new QPushButton("🎙️", window);
    stopButton = new QPushButton("⛔", window);

    for (int j = 0; j < (int)maxmes; j++)
    {
        QString *qstr = new QString();
        yqstr.push_back(qstr);
    }

    for (int m = 0; m < (int)maxmes; m++)
    {
        QLabel *label = new QLabel(*yqstr[m], window);
        label->setGeometry(200, 30*m, 250, 60);

        QPalette palette2 = label->palette();
        palette2.setColor (QPalette:: WindowText ,Qt::black);
        label->setPalette(palette2);

        ylabels.push_back(label);
    }

    for (int i = 0; i < (int)maxmes; i++)
    {
        QString *Qstr = new QString();
        oqstr.push_back(Qstr);
    }

    for (int n = 0; n < (int)maxmes; n++)
    {
        QLabel *Label = new QLabel(*oqstr[n], window);
        Label->setGeometry(20, 30*n, 250, 60);

        QPalette palette3 = Label->palette();
        palette3.setColor (QPalette:: WindowText ,Qt::black);
        Label->setPalette(palette3);

        olabels.push_back(Label);
    }

}

void windows :: setgeometry()
{
    window -> setFixedSize(WindowSizeX, WindowSizeY);

    Sending -> setGeometry(WindowSizeX - ButSendingSizeX, WindowSizeY - ButSendingSizeY - 20, ButSendingSizeX, ButSendingSizeY);

    // Создаем палитру
    QPalette palette = Sending->palette();
    palette.setColor (QPalette:: Button,QColor ("#00CCFF")); // Цвет фона
    palette.setColor(QPalette::ButtonText, Qt::white);
    // Цвет текста
    Sending->setPalette(palette);

    Texting -> setGeometry(20, WindowSizeY - ButSendingSizeY - 20, LbTextingSizeX, LbTextingSizeY);

    QLabel *background = new QLabel (window);
    QPixmap *pixmap = new QPixmap("D:/fon.JPG");
    background->setPixmap (pixmap->scaled (window->size(), Qt ::KeepAspectRatioByExpanding));
    background->lower();

    startButton -> setGeometry(WindowSizeX - 50 - (2 * ButSendingSizeX), WindowSizeY - ButSendingSizeY - 20,ButSendingSizeX, ButSendingSizeY);
    stopButton -> setGeometry(WindowSizeX - 50 - ButSendingSizeX, WindowSizeY - ButSendingSizeY - 20, ButSendingSizeX, ButSendingSizeY);
}

void windows :: showing()
{
    window -> show();
}

void windows :: connecting()
{
    connect(Sending,SIGNAL(clicked()),SLOT(sendingslot()));
    connect(startButton,SIGNAL(clicked()),SLOT(startRecording()));
    connect(stopButton,SIGNAL(clicked()),SLOT(stopRecording()));
}

void windows :: sendingslot()
{
    message = Texting->text();
    sendStr = message.toStdString();

    answ = true;
}

void windows :: startRecording()
{
    if (!recorder) return;

    // Устанавливаем выходной файл
    QString type("file:");
    QString name("message");
    QDate date=QDate::currentDate();
    QTime time=QTime::currentTime();
    QString datatime = date.toString("yyyy_MM_dd") + time.toString("_hh_mm_ss_ms");
    QString format = ".wav";
    inputFile = name+datatime+format;
    recorder->setOutputLocation(QUrl::fromLocalFile(type+inputFile));

    // Начинаем запись
    recorder->record();
    qDebug() << "Recording started...";
}

void windows :: stopRecording() {
    if (!recorder) return;

    // Останавливаем запись
    recorder->stop();
    qDebug() << "Recording finished!";

    QFile file(inputFile);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "FILE ERROR:" << file.errorString();
        return;
    }

    QByteArray wavData = file.readAll();
    file.close();
    audioMessage.assign(wavData.begin(), wavData.end());

    if (!audioMessage.isEmpty()) {
        qDebug() << "conver have been completed" << audioMessage.length();
        sendSTR = markAUD; //+ audioMessage.toStdString();
        answ = true;
    }
}

void windows :: messages()
{
    while (ob.isWorking())
    {
        std::this_thread::sleep_for(100ms);
        const std::string quit = "quit";

        if (sendStr == quit)
        {
            std::cout << "Finishing the work" << std::endl;
            result = shutdown(ConnectSocket, SD_SEND);
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            ob.shutdown();
            T.join();
            T2.join();
            T3.join();
        }

        if(y == true && position < maxmes)
        {
            *yqstr[position] = QString::fromStdString(sendStr2);
            ylabels[position]->setText(*yqstr[position]);

            *oqstr[position] = "";
            olabels[position]->setText(*oqstr[position]);

            position += 1;

            y = false;

            Texting->setText("");
        }

        else if(o == true && position < maxmes)
        {
            *oqstr[position] = QString::fromStdString(recvStr2);
            olabels[position]->setText(*oqstr[position]);

            *yqstr[position] = "";
            ylabels[position]->setText(*yqstr[position]);

            position += 1;

            o = false;
        }

        else if (y == true && position >= maxmes)
        {
            for (int i = 0; i < position-1; i++)
            {
                *yqstr[i] = *yqstr[i+1];
                ylabels[i]->setText(*yqstr[i]);

                *oqstr[i] = *oqstr[i+1];
                olabels[i]->setText(*oqstr[i]);
            }

            *yqstr[maxmes - 1] = QString::fromStdString(sendStr2);
            ylabels[maxmes - 1]->setText(*yqstr[maxmes - 1]);

            *oqstr[maxmes - 1] = QString::fromStdString("");
            olabels[maxmes - 1]->setText(*oqstr[maxmes - 1]);

            y = false;

            Texting->setText("");
        }

        else if(o == true && position >= maxmes)
        {
            for (int h = 0; h < position-1; h++)
            {
                *oqstr[h] = *oqstr[h+1];
                olabels[h]->setText(*oqstr[h]);

                *yqstr[h] = *yqstr[h+1];
                ylabels[h]->setText(*yqstr[h]);
            }

            *oqstr[maxmes - 1] = QString::fromStdString(recvStr2);
            olabels[maxmes - 1]->setText(*oqstr[maxmes - 1]);

            *yqstr[maxmes - 1] = QString::fromStdString("");
            ylabels[maxmes - 1]->setText(*yqstr[maxmes - 1]);

            o = false;
        }
    }
}

QString windows::converttoascii(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "FILE ERROR:" << file.errorString();
        return "";
    }

    QByteArray wavData = file.readAll();
    file.close();

    QString hexString;
    hexString.reserve(wavData.size() * 2); // Оптимизация памяти

    for (char byte : wavData)
    {
        hexString.append(QString("%1").arg((quint8)byte, 2, 16, QChar('0')));
    }

    return hexString;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setlocale(LC_ALL, "Russian");

    body ob;
    windows obj{ob};

    obj.creating();
    std::thread T3([&](){obj.messages();});
    obj.setupSettings();
    obj.setgeometry();
    obj.connecting();
    obj.showing();

    auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        std::cout << "Mistake with WSAStartup" << std::endl;

        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = getaddrinfo("localhost", "2626", &hints, &addrResult);

    if (result != 0)
    {
        std::cout << "Mistake with getaddrinfo" << std::endl;
        WSACleanup();
        return 1;
    }

    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "Mistake with creating socket" << std::endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);

    if (result == SOCKET_ERROR)
    {
        std::cout << "Mistake with connecting to server" << std::endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }



    // name();
    std::thread T2([&](){ob.doing();});

    a.exec();

    T2.join();
    T3.join();

    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}

//#include "main.moc"
