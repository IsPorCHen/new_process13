#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

#define PIPE_NAME L"\\\\.\\pipe\\MyPipe"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");

    HANDLE hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 256, 256, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "[Родительский процесс] Ошибка создания канала! Код ошибки: " << GetLastError() << endl;
        return 1;
    }

    cout << "[Родительский процесс] Канал создан, ожидаем подключения дочернего процесса..." << endl;

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(L"C:\\Users\\ipch\\source\\repos\\new_process13\\x64\\Debug\\child.exe", NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        cerr << "[Родительский процесс] Ошибка создания дочернего процесса!" << GetLastError() <<endl;
        CloseHandle(hPipe);
        return 1;
    }

    if (!ConnectNamedPipe(hPipe, NULL)) {
        cerr << "[Родительский процесс] Ошибка подключения к каналу! Код ошибки: " << GetLastError() << endl;
        CloseHandle(hPipe);
        return 1;
    }

    cout << "[Родительский процесс] Дочерний процесс подключен." << endl;

    string command;
    while (true) {
        cout << "[Родительский процесс] Введите команду (1 <количество> - создание потоков\n2 <id> - удаление потока\n3 - выход): ";
        getline(cin, command);

        DWORD bytesWritten;
        WriteFile(hPipe, command.c_str(), command.size() + 1, &bytesWritten, NULL);

        if (command == "3") {
            break;
        }

        Sleep(500);
    }

    CloseHandle(hPipe);
    cout << "[Родительский процесс] Завершение работы." << endl;

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
