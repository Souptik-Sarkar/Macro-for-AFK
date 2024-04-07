#include <windows.h>
#include <stdio.h>

#define ID_BUTTON_RECORD 101
#define ID_BUTTON_PLAY   102
#define ID_BUTTON_CLOSE  103

BOOL isRecording = FALSE;
BOOL isPlaying = FALSE;

// Function to record macro
void recordMacro(HWND hwnd) {
    MessageBox(hwnd, "Recording macro...\nPress 'ESC' to stop recording.", "Recording", MB_OK);

    isRecording = TRUE;

    FILE *fp = fopen("macroV2.txt", "w");
    if (fp == NULL) {
        MessageBox(hwnd, "Error opening file for writing.", "Error", MB_OK | MB_ICONERROR);
        isRecording = FALSE;
        return;
    }

    DWORD startTime = GetTickCount(); // Get current time

    while (isRecording) { // Record until 'ESC' is pressed
        for (char c = 8; c <= 255; c++) {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { // Check for 'ESC' key
                isRecording = FALSE;
                MessageBox(hwnd, "Recording stopped", "Info", MB_OK);
                break;
            }
            if (GetAsyncKeyState(c) & 0x8000) { // Key is pressed
                DWORD elapsedTime = GetTickCount() - startTime; // Calculate elapsed time
                fprintf(fp, "%c>%d:", c, elapsedTime);
                startTime = GetTickCount(); // Reset start time
                break;
            }
        }
        Sleep(100); // Delay to prevent recording too fast
    }

    fclose(fp);
}

// Function to play macro
void playMacro(HWND hwnd) {
    MessageBox(hwnd, "Playing macro...\nPress 'ESC' to stop playing.", "Playing", MB_OK);

    isPlaying = TRUE;

    FILE *fp = fopen("macroV2.txt", "r");
    if (fp == NULL) {
        MessageBox(hwnd, "Error opening file for reading.", "Error", MB_OK | MB_ICONERROR);
        isPlaying = FALSE;
        return;
    }

    char key;
    int delay;
    while (isPlaying) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { // Check for 'ESC' key
            isPlaying = FALSE;
            MessageBox(hwnd, "Playback stopped", "Info", MB_OK);
            break;
        }

        fseek(fp, 0, SEEK_SET); // Move file pointer to the beginning
        while (fscanf(fp, "%c>%d:", &key, &delay) != EOF && isPlaying) {
            // Simulate keypress using SendInput
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = key; // Virtual-key code for the key
            input.ki.dwFlags = 0; // Key press
            SendInput(1, &input, sizeof(INPUT));

            Sleep(delay); // Wait for delay

            input.ki.dwFlags = KEYEVENTF_KEYUP; // Key release
            SendInput(1, &input, sizeof(INPUT));

            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { // Check for 'ESC' key
                isPlaying = FALSE;
                MessageBox(hwnd, "Playback stopped", "Info", MB_OK);
                break;
            }
        }
    }

    fclose(fp);

    isPlaying = FALSE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    const char CLASS_NAME[] = "MacroWindowClass";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        "Scars Ultimate Macro",        // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 200,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Create labels
    CreateWindow(
        "STATIC",                       // Predefined class; Unicode assumed
        "Click 'Record' to start recording, 'Play' to play recorded macro, and 'Close' to exit the application.",  // Text
        WS_VISIBLE | WS_CHILD | SS_CENTER,  // Styles
        10,                             // x position
        10,                             // y position
        480,                            // Width
        30,                             // Height
        hwnd,                           // Parent window
        NULL,                           // No menu
        hInstance,                      // Instance handle
        NULL                            // No additional parameter
    );

    // Create buttons
    CreateWindow(
        "BUTTON",                   // Predefined class; Unicode assumed
        "Record",                   // Button text
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
        50,                         // x position
        50,                         // y position
        100,                        // Button width
        50,                         // Button height
        hwnd,                       // Parent window
        (HMENU)ID_BUTTON_RECORD,   // Menu ID
        hInstance,
        NULL                        // Pointer not needed.
    );

    CreateWindow(
        "BUTTON",
        "Play",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        190,
        50,
        100,
        50,
        hwnd,
        (HMENU)ID_BUTTON_PLAY,
        hInstance,
        NULL
    );

    CreateWindow(
        "BUTTON",
        "Close",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        330,
        50,
        100,
        50,
        hwnd,
        (HMENU)ID_BUTTON_CLOSE,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_RECORD:
                    if (!isRecording) {
                        // Start recording
                        recordMacro(hwnd);
                    } else {
                        // Stop recording
                        isRecording = FALSE;
                    }
                    break;
                case ID_BUTTON_PLAY:
                    if (!isPlaying) {
                        // Start playing
                        playMacro(hwnd);
                    } else {
                        // Stop playing
                        isPlaying = FALSE;
                    }
                    break;
                case ID_BUTTON_CLOSE:
                    // Close the application
                    DestroyWindow(hwnd);
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

