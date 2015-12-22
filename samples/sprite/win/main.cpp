// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <windows.h>
#include <windowsx.h>
#include "../Application.h"


using namespace ouzel;

extern HWND GMainWindow;


static Engine* getEngine(HWND window)
{
    return (Engine*) GetWindowLongPtrW(window, GWLP_USERDATA);
}

static void mouseButtonEvent(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool isDown = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
    int button = 0;
    if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
    {
        button = 1;
    }
    else if(msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
    {
        button = 2;
    }
    Engine* engine = getEngine(window);
    Vector2 pos((float) GET_X_LPARAM(lParam), (float) GET_Y_LPARAM(lParam));
    
    Event event;
    memset(&event, 0, sizeof(event));
    event.type = isDown ? EventType::EVENT_TYPE_MOUSE_DOWN : EventType::EVENT_TYPE_MOUSE_UP;
    event.mouseButton = button;
    event.mousePosition = engine->getRenderer()->absoluteToWorldLocation(pos);
    if(wParam & MK_SHIFT)
    {
        event.shiftDown = true;
    }
    if(wParam & MK_CONTROL)
    {
        event.controlDown = true;
    }
    engine->handleEvent(event);
}

static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_KEYUP:
        break;
    case WM_KEYDOWN:
        break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        mouseButtonEvent(window, msg, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"OuzelWindow";
    RegisterClassExW(&wc);
    
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT windowRect = {0, 0, 640, 480};
    AdjustWindowRect(&windowRect, style, FALSE);
    
    HWND window = CreateWindowExW(
        NULL,
        L"OuzelWindow",
        L"Ouzel",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hInstance,
        NULL);
	GMainWindow = window;
    ouzel::Application application;
	SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)application.getEngine());
	ShowWindow(window, SW_SHOW);
    
    bool run = true;
    while(run)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT)
            {
                run = false;
                break;
            }
        }
        if(run == false)
        {
            break;
        }
        
        application.getEngine()->run();
    }
    
    DestroyWindow(window);
    
    return 0;
}

