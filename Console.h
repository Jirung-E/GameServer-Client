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
        // 콘솔창 닫기
        HWND hwndConsole = GetConsoleWindow();  // 현재 콘솔 창의 핸들 가져오기
        if(hwndConsole) {
            ShowWindow(hwndConsole, SW_HIDE);  // 콘솔 창 숨기기
            FreeConsole();
        }
    }
};