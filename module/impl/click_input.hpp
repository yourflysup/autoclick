#pragma once

#include "../module/module.h"

#include <Windows.h>
#include <atomic>
#include <random>
#include <thread>
#include <intrin.h>
#include <mutex>

class click_input final : public module {
public:
    static inline click_input* instance{ nullptr };

    HHOOK hook{ nullptr };
    std::atomic_bool mouseDown{ false };
    std::atomic_bool running{ true };
    bool avoidMenu{ false };

    click_input() : module("Yourflysup") {
        instance = this;
        timeBeginPeriod(1);

        CreateThread(nullptr, 0, HookThread, nullptr, 0, nullptr);
        std::thread(&click_input::Clicker, this).detach();
    }

    ~click_input() override {
        running.store(false, std::memory_order_relaxed);
        timeEndPeriod(1);
    }

private:
    static LRESULT CALLBACK MouseProc(int code, WPARAM wp, LPARAM lp) {
        if (code == HC_ACTION) {
            const auto* m = reinterpret_cast<MSLLHOOKSTRUCT*>(lp);
            if (m && !(m->flags & LLMHF_INJECTED)) {
                if (wp == WM_LBUTTONDOWN) instance->mouseDown.store(true, std::memory_order_relaxed);
                if (wp == WM_LBUTTONUP)   instance->mouseDown.store(false, std::memory_order_relaxed);
            }
        }
        return CallNextHookEx(instance->hook, code, wp, lp);
    }

    static DWORD WINAPI HookThread(LPVOID) {
        instance->hook = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, nullptr, 0);

        MSG msg{};
        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        UnhookWindowsHookEx(instance->hook);
        return 0;
    }

    static bool CursorVisible() {
        CURSORINFO ci{ sizeof(ci) };
        return GetCursorInfo(&ci) && (ci.flags & CURSOR_SHOWING);
    }

    static void Click(int cps) {
        static LARGE_INTEGER freq{};
        static std::once_flag once;

        std::call_once(once, [] {
            QueryPerformanceFrequency(&freq);
        });

        LARGE_INTEGER start{}, now{};
        QueryPerformanceCounter(&start);

        const double periodMs = 1000.0 / static_cast<double>(cps);
        const double halfMs   = periodMs * 0.5;

        INPUT down{ INPUT_MOUSE };
        down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &down, sizeof(INPUT));

        do {
            QueryPerformanceCounter(&now);
            _mm_pause();
        } while (
            (static_cast<double>(now.QuadPart - start.QuadPart) * 1000.0) /
            static_cast<double>(freq.QuadPart) < halfMs
        );

        INPUT up{ INPUT_MOUSE };
        up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &up, sizeof(INPUT));

        do {
            QueryPerformanceCounter(&now);
            _mm_pause();
        } while (
            (static_cast<double>(now.QuadPart - start.QuadPart) * 1000.0) /
            static_cast<double>(freq.QuadPart) < periodMs
        );
    }

    void Clicker() const {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        std::mt19937 rng{ std::random_device{}() };

        while (running.load(std::memory_order_relaxed)) {
            if (!status) {
                Sleep(1);
                continue;
            }

            if (avoidMenu && CursorVisible()) {
                Sleep(1);
                continue;
            }

            if (mouseDown.load(std::memory_order_relaxed)) {
                std::uniform_int_distribution<int> cps(minCPS, maxCPS);
                Click(cps(rng));
            } else {
                Sleep(1);
            }
        }
    }
};