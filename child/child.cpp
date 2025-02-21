#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

#define PIPE_NAME L"\\\\.\\pipe\\MyPipe"

HANDLE mutex;
unordered_map<int, HANDLE> threads;
unordered_map<int, bool> threadRunning;
int threadCount = 0;

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    int id = *(int*)lpParam;
    srand(time(0) + id);
    int a = rand() % 100;
    int b = rand() % 100;
	WaitForSingleObject(mutex, INFINITE);
    cout << "[Поток " << id << "] Сумма: " << a << " + " << b << " = " << (a + b) << endl;
	ReleaseMutex(mutex);
	Sleep(2000);
    return 0;
}

void CreateThreads(int count) {
    for (int i = 0; i < count; i++) {
		threadCount++;
		threadRunning[threadCount] = true;
        int* threadID = new int(i + 1);
        HANDLE hThread = CreateThread(NULL, 0, ThreadFunction, threadID, 0, NULL);
        if (hThread) {
			WaitForSingleObject(mutex, INFINITE);
            cout << "[Дочерний процесс] Создан поток " << (*threadID) << endl;
			ReleaseMutex(mutex);
            threads[threadCount] = hThread;
        }
    }
}

void DeleteThread(int id) {
	if (threads.find(id) != threads.end())
	{
		threadRunning[id] = false;

		WaitForSingleObject(mutex, INFINITE);
		CloseHandle(threads[id]);

		threads.erase(id);
		threadRunning.erase(id);
		WaitForSingleObject(mutex, INFINITE);
		cout << "[Дочерний процесс] Поток " << id << " удален." << endl;
		ReleaseMutex(mutex);

	}
	else {
		cout << "[Дочерний процесс] Ошибка, неверный id потока: " << GetLastError() ;
	}
	//if (id > 0 && id <= threads.size()) {
 //       TerminateThread(threads[id - 1], 0);
 //       CloseHandle(threads[id - 1]);
 //       threads.erase(threads.begin() + id - 1);
 //       cout << "[Дочерний процесс] Поток " << id << " удален." << endl;
 //   }
}

int main() {
    setlocale(0, "rus");
    HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "[Дочерний процесс] Ошибка подключения к каналу! Код ошибки: " << GetLastError() << endl;
        return 1;
    }

    cout << "[Дочерний процесс] Подключение к каналу успешно." << endl;

    char buffer[256];
    while (true) {
		int count = 0;
        DWORD bytesRead;
        if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            string command(buffer);

            if (!command.empty()) {
                cout << "[Дочерний процесс] Получена команда: " << command << endl;

                if (command[0] == '1') {
                    count = stoi(command.substr(2));
                    CreateThreads(count);
                }
                else if (command[0] == '2') {
                    int id = stoi(command.substr(2));
					DeleteThread(id);
                }
                else if (command[0] == '3') {
                    break;
                }
            }
        }
	
		memset(buffer, 0, sizeof(buffer));
        Sleep(500);
    }

	for (auto it = threads.begin(); it != threads.end(); ++it) {
		WaitForSingleObject(mutex, INFINITE);
		CloseHandle(it->second);
	}

    //for (HANDLE thread : threads) {
    //    TerminateThread(thread, 0);
    //    CloseHandle(thread);
    //}

    CloseHandle(hPipe);
    cout << "[Дочерний процесс] Завершение работы." << endl;
    return 0;
}
