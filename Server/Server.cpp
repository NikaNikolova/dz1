#define WIN32_LEAN_AND_MEAN 

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
	setlocale(0, "");
	system("title SERVER SIDE");
	cout << "процесс сервера запущен!\n";

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed with error: " << iResult << "\n";
		cout << "подключение Winsock.dll прошло с ошибкой!\n";
		return 1;
	}

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed with error: " << iResult << "\n";
		cout << "получение адреса и порта сервера прошло c ошибкой!\n";
		WSACleanup();
		return 2;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "socket failed with error: " << WSAGetLastError() << "\n";
		cout << "создание сокета прошло c ошибкой!\n";
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << "\n";
		cout << "внедрение сокета по IP-адресу прошло с ошибкой!\n";
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cout << "listen failed with error: " << WSAGetLastError() << "\n";
		cout << "прослушка информации от клиента не началась. что-то пошло не так!\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed with error: " << WSAGetLastError() << "\n";
		cout << "соединение с клиентским приложением не установлено! печаль!\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 6;
	}

	closesocket(ListenSocket);

	//////////////////////////////////////////////////////////

   /* while (true) {
		SOCKET ClientSocket = INVALID_SOCKET;
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			cout << "accept failed with error: " << WSAGetLastError() << "\n";
			cout << "соединение с клиентским приложением не установлено! печаль!\n";
			closesocket(ListenSocket);
			WSACleanup();
			return 6;
		}*/

		// Process client messages
	do {
		char clientMessage[DEFAULT_BUFLEN];
		iResult = recv(ClientSocket, clientMessage, DEFAULT_BUFLEN, 0);

		if (iResult > 0) {
			clientMessage[iResult] = '\0';
			cout << "Клиент отправил сообщение: " << clientMessage << "\n";

			// Определение ответа на команду клиента
			string response;
			string command = clientMessage;

			if (command == "как дела?") {
				response = "отлично";
			}
			else if (command == "привет") {
				response = "привет!";
			}
			else if (command == "пока") {
				response = "пока!";
			}
			else if (command == "как погода?") {
				response = "так себе(";
			}
			else if (command == "расскажи новость") {
				response = "коты и собаки были одним видом, но в процессе эволюции разбились на два разных вида";
			}
			else if (command == "что делаешь?") {
				response = "делаю мир счастливым)";
			}
			else if (command == "как тебя зовут?") {
				response = "Server v1)";
			}
			else if (command == "расскажи анекдот") {
				response = "Как программист объясняет своему другу, что он слишком много работает?\n — Ну, в общем, я бы сказал, что у меня ошибка 404 в социальной жизни – друзей не найдено!)";
			}
			else if (command == "как тебя зовут?") {
				response = "Server v1)";
			}
			else if (command == "скажи время") 
			{
				// Получение текущего времени
				time_t now = time(0);
				tm localTime;
				if (localtime_s(&localTime, &now) == 0) 
				{
					char timeBuffer[80];
					if (strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %X", &localTime) > 0) 
					{
						response = timeBuffer;
					}
					else 
					{
						response = "Ошибка форматирования времени";
					}
				}
				else 
				{
					response = "Ошибка при получении текущего времени";
				}
			}
			else if (command == "выход")
			{
				response = "До свидания! Завершаем соединение.";
			}
				// отправляем ответ клиенту
			const char* serverResponse = response.c_str();
			iResult = send(ClientSocket, serverResponse, strlen(serverResponse), 0);

			if (iResult == SOCKET_ERROR) {
				cout << "send failed with error: " << WSAGetLastError() << "\n";
				closesocket(ClientSocket);
				WSACleanup();
				return 7;
			}

			cout << "Ответ отправлен клиенту: " << response << "\n";
			break;
		}
		
		else if (iResult == 0) {
			cout << "Соединение с клиентом закрыто.\n";
			break;
		}
		else {
			cout << "recv failed with error: " << WSAGetLastError() << "\n";
			closesocket(ClientSocket);
			WSACleanup();
			return 8;
		}

	} while (iResult > 0);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		cout << "shutdown failed with error: " << WSAGetLastError() << "\n";
		cout << "упс, разрыв соединения (shutdown) выдал ошибку ((\n";
	}
	closesocket(ClientSocket);
	WSACleanup();
	cout << "Процесс сервера прекращает свою работу! До новых запусков! :)\n";
	return 0;

}