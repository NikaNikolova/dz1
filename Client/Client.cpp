// https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1000

int main(int argc, char** argv) // имя сервера при желании можно будет указать через параметры командной строки
{
    // я попытался максимально упростить запуск клиентского приложения, поэтому количество параметров командной строки не проверяется!
    // Validate the parameters
    //if (argc != 2) {
    //    printf("usage: %s server-name\n", argv[0]);
    //    return 10;
    //}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    setlocale(0, "");
    system("title CLIENT SIDE");
    //cout << "процесс клиента запущен!\n";
    //Sleep(PAUSE);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        return 11;
    }
    else {
        //cout << "подключение Winsock.dll прошло успешно!\n";
        //Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    const char* ip = "localhost"; // по умолчанию, оба приложения, и клиент, и сервер, запускаются на одной и той же машине

    //iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result); // раскомментировать, если нужно будет читать имя сервера из командной строки
    addrinfo* result = NULL;
    iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << "\n";
        WSACleanup();
        return 12;
    }
    else {
        //cout << "получение адреса и порта клиента прошло успешно!\n";
        //Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Attempt to connect to an address until one succeeds
    SOCKET ConnectSocket = INVALID_SOCKET;

    for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) { // серверов может быть несколько, поэтому не помешает цикл

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 13;
        }

        // Connect to server
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            // cout << "Connection with server failed\n";
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        //cout << "создание сокета на клиенте прошло успешно!\n";
        //Sleep(PAUSE);

        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "невозможно подключиться к серверу. убедитесь, что процесс сервера запущен!\n";
        WSACleanup();
        return 14;
    }
    else {
        //cout << "подключение к серверу прошло успешно!\n";
        //Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    while (true)
    {
        string commands[] = { "привет","как дела?", "как погода?", "расскажи новость", "пока", "что делаешь?", "скажи время", "как тебя зовут?", "расскажи анекдот", "выход" };

        cout << "Список команд клиента:\n";
        for (int i = 0; i < 10; ++i) {
            cout << i + 1 << ". " << commands[i] << "\n";
        }

        do {
            // Получение команды от пользователя
            cout << "Введите номер команды (1-10): ";
            int commandNumber;
            cin >> commandNumber;
            cin.ignore();  // Очистка буфера ввода

            // Проверка введенного номера команды
            if (commandNumber < 1 || commandNumber > 10) {
                cout << "Неверный номер команды. Пожалуйста, введите от 1 до 10.\n";
                continue;
            }

            // Отправка команды на сервер
            const char* message = commands[commandNumber - 1].c_str();
            iResult = send(ConnectSocket, message, strlen(message), 0);

            if (iResult == SOCKET_ERROR) {
                cout << "send failed with error: " << WSAGetLastError() << "\n";
                closesocket(ConnectSocket);
                WSACleanup();
                return 15;
            }

            // Получение ответа от сервера
            char answer[DEFAULT_BUFLEN];
            iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);

            if (iResult > 0) {
                answer[iResult] = '\0';
                cout << "Ответ от сервера: " << answer << "\n";
                cout << "Байтов получено: " << iResult << "\n";
                if (strcmp(message, "скажи время") == 0) {
                    cout << "Текущее время: " << answer << "\n";
                }
            }
            else if (iResult == 0) {
                cout << "Соединение с сервером закрыто.\n";
            }
            else {
                cout << "recv failed with error: " << WSAGetLastError() << "\n";
            }

        } while (true);

        // Send an initial buffer
        // const char* message = "hello from client!";
        string message;
        cout << "Please, enter some message to server: ";
        // cin >> message; // считывание строки происходит до первого пробела 
        getline(cin, message);

        iResult = send(ConnectSocket, message.c_str(), (int)message.size(), 0);
        if (iResult == SOCKET_ERROR) {
            cout << "send failed with error: " << WSAGetLastError() << "\n";
            closesocket(ConnectSocket);
            WSACleanup();
            return 15;
        }
        else {
            //cout << "клиент отправил сообщение: " << message << "\n";
            //cout << "байтов с клиента отправлено: " << iResult << "\n";
            //Sleep(PAUSE);
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////

        // shutdown the connection since no more data will be sent
        /*
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            cout << "shutdown failed with error: " << WSAGetLastError() << "\n";
            closesocket(ConnectSocket);
            WSACleanup();
            return 16;
        }
        else {
            cout << "процесс клиента инициирует закрытие соединения с сервером.\n";
        }*/

        /////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Receive until the peer closes the connection
        char answer[DEFAULT_BUFLEN];

        //do {

        iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
        answer[iResult] = '\0';

        if (iResult > 0) {
            cout << "сервер ответил: " << answer << "\n";
            // cout << "байтов получено: " << iResult << "\n";
        }
        else if (iResult == 0)
            cout << "соединение с сервером закрыто.\n";
        else
            cout << "recv failed with error: " << WSAGetLastError() << "\n";

        // } while (iResult > 0);

    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    //cout << "процесс клиента прекращает свою работу!\n";

    // return 0;
}