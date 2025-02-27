#include <windows.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

#define PIPE_NAME L"\\\\.\\pipe\\MyPipe"

HANDLE mutex;
unordered_map<int, HANDLE> threads;
unordered_map<int, bool> threadRunning;
int threadCount = 0;

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    int id = *(int*)lpParam;
    srand(GetTickCount() + id);

    while (true) {
        if (!threadRunning[id]) break;

        int a = rand() % 100;
        int b = rand() % 100;

        WaitForSingleObject(mutex, INFINITE);
        cout << "[Поток " << id << "] Сумма: " << a << " + " << b << " = " << (a + b) << endl;
        ReleaseMutex(mutex);

        Sleep(2000);
    }

    WaitForSingleObject(mutex, INFINITE);
    cout << "[Поток " << id << "] Завершается." << endl;
    ReleaseMutex(mutex);

    return 0;
}

void CreateThreads(int count) {
    for (int i = 0; i < count; i++) {
        threadCount++;
        threadRunning[threadCount] = true;

        int* threadID = new int(threadCount);
        HANDLE hThread = CreateThread(NULL, 0, ThreadFunction, threadID, 0, NULL);

        if (hThread) {
            WaitForSingleObject(mutex, INFINITE);
            cout << "[Дочерний процесс] Создан поток " << threadCount << endl;
            ReleaseMutex(mutex);

            threads[threadCount] = hThread;
        }
        else {
            delete threadID;
        }
    }
}

void DeleteThread(int id) {
    if (threads.find(id) != threads.end()) {
        threadRunning[id] = false;
        WaitForSingleObject(threads[id], INFINITE);
        CloseHandle(threads[id]);

        WaitForSingleObject(mutex, INFINITE);
        cout << "[Дочерний процесс] Поток " << id << " удален." << endl;
        ReleaseMutex(mutex);

        threads.erase(id);
        threadRunning.erase(id);
    }
    else {
        WaitForSingleObject(mutex, INFINITE);
        cout << "[Дочерний процесс] Ошибка: поток с ID " << id << " не найден." << endl;
        ReleaseMutex(mutex);
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    mutex = CreateMutex(NULL, FALSE, NULL);

    HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "[Дочерний процесс] Ошибка подключения к каналу! Код ошибки: " << GetLastError() << endl;
        return 1;
    }

    cout << "[Дочерний процесс] Подключение к каналу успешно." << endl;

    char buffer[256];
    while (true) {
        DWORD bytesRead;
        if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            string command(buffer);

            if (!command.empty()) {
                cout << "[Дочерний процесс] Получена команда: " << command << endl;

                if (command[0] == '1') {
                    int count = stoi(command.substr(2));
                    CreateThreads(count);
                }
                else if (command[0] == '2') {
                    int id = stoi(command.substr(2));
                    DeleteThread(id);
                }
            }
        }

        memset(buffer, 0, sizeof(buffer));
        Sleep(500);
    }

    for (auto it = threads.begin(); it != threads.end(); ++it) {
        threadRunning[it->first] = false;
        WaitForSingleObject(it->second, INFINITE);
        CloseHandle(it->second);
    }

    CloseHandle(hPipe);
    CloseHandle(mutex);
    cout << "[Дочерний процесс] Завершение работы." << endl;
    return 0;
}
