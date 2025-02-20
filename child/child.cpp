#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define PIPE_NAME L"\\\\.\\pipe\\MyPipe"

vector<HANDLE> threads;

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    int id = *(int*)lpParam;
    srand(GetTickCount() + id);
    int a = rand() % 100;
    int b = rand() % 100;
    cout << "[Поток " << id << "] Сумма: " << a << " + " << b << " = " << (a + b) << endl;
    return 0;
}

void CreateThreads(int count) {
    for (int i = 0; i < count; i++) {
        int* threadID = new int(i + 1);
        HANDLE hThread = CreateThread(NULL, 0, ThreadFunction, threadID, 0, NULL);
        if (hThread) {
            cout << "[Дочерний процесс] Создан поток " << (*threadID) << endl;
            threads.push_back(hThread);
        }
    }
}

void DeleteThread(int id) {
    if (id > 0 && id <= threads.size()) {
        TerminateThread(threads[id - 1], 0);
        CloseHandle(threads[id - 1]);
        threads.erase(threads.begin() + id - 1);
        cout << "[Дочерний процесс] Поток " << id << " удален." << endl;
    }
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
                else if (command[0] == '3') {
                    break;
                }
            }
        }

        memset(buffer, 0, sizeof(buffer)); // Очищаем буфер после обработки
        Sleep(500);
    }

    // Очистка
    for (HANDLE thread : threads) {
        TerminateThread(thread, 0);
        CloseHandle(thread);
    }

    CloseHandle(hPipe);
    cout << "[Дочерний процесс] Завершение работы." << endl;
    return 0;
}
