#pragma once


class Console {
public:
    Console() {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    }
    ~Console() {
        close();
    }

public:
    void close() {
        // �ܼ�â �ݱ�
        HWND hwndConsole = GetConsoleWindow();  // ���� �ܼ� â�� �ڵ� ��������
        if(hwndConsole) {
            ShowWindow(hwndConsole, SW_HIDE);  // �ܼ� â �����
            FreeConsole();
        }
    }
};