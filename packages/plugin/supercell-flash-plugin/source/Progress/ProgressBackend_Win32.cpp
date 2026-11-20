#include "ProgressBackend.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <string>

namespace sc::Adobe {
    namespace {
        constexpr wchar_t WINDOW_CLASS[] = L"SCAnimateProgress";

        class ProgressBackendWin32 final : public IProgressBackend {
        public:
            static ProgressBackendWin32* s_instance;

            ~ProgressBackendWin32() override { Destroy(); }

            bool Create(const char* title, ExportProgress* progress) override {
                if (m_hwnd)
                    return true;

                m_progress = progress;
                ImGui_ImplWin32_EnableDpiAwareness();

                WNDCLASSEXW wc = {};
                wc.cbSize = sizeof(wc);
                wc.lpfnWndProc = WndProc;
                wc.hInstance = GetModuleHandleW(nullptr);
                wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
                wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
                wc.lpszClassName = WINDOW_CLASS;
                RegisterClassExW(&wc);

                constexpr DWORD exStyle = WS_EX_DLGMODALFRAME | WS_EX_TOPMOST;
                constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;

                RECT desiredClient = {0, 0, (LONG) PROGRESS_WINDOW_WIDTH, (LONG) PROGRESS_WINDOW_HEIGHT};
                AdjustWindowRectEx(&desiredClient, style, FALSE, exStyle);

                float dpiScale =
                    ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint(POINT {0, 0}, MONITOR_DEFAULTTOPRIMARY));
                int width = (int) ((desiredClient.right - desiredClient.left) * dpiScale);
                int height = (int) ((desiredClient.bottom - desiredClient.top) * dpiScale);

                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                int x = (screenWidth - width) / 2;
                int y = (screenHeight - height) / 2;

                m_hwnd = CreateWindowExW(exStyle,
                                         WINDOW_CLASS,
                                         Utf8ToWide(title).c_str(),
                                         style,
                                         x,
                                         y,
                                         width,
                                         height,
                                         nullptr,
                                         nullptr,
                                         GetModuleHandleW(nullptr),
                                         this);
                if (!m_hwnd)
                    return false;

                if (!CreateDevice())
                    return false;

                ImGui_ImplWin32_Init(m_hwnd);
                ImGui_ImplDX11_Init(m_device, m_context);
                m_backendInitialized = true;
                ResizeRenderTarget();
                return true;
            }

            void BeginFrame() override {
                MSG msg;
                while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }

                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
            }

            void EndFrame() override {
                ImGui::Render();

                if (m_renderTarget) {
                    const float clearColor[4] = {0.16f, 0.16f, 0.16f, 1.0f};
                    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
                    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
                }
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                if (m_swapChain)
                    m_swapChain->Present(1, 0);
            }

            bool IsOpen() const override { return m_hwnd != nullptr && !m_closeRequested; }

            void Destroy() override {
                if (m_backendInitialized) {
                    ImGui_ImplDX11_Shutdown();
                    ImGui_ImplWin32_Shutdown();
                    m_backendInitialized = false;
                }

                if (m_renderTarget) {
                    m_renderTarget->Release();
                    m_renderTarget = nullptr;
                }
                if (m_swapChain) {
                    m_swapChain->Release();
                    m_swapChain = nullptr;
                }
                if (m_context) {
                    m_context->Release();
                    m_context = nullptr;
                }
                if (m_device) {
                    m_device->Release();
                    m_device = nullptr;
                }

                if (m_hwnd) {
                    DestroyWindow(m_hwnd);
                    m_hwnd = nullptr;
                }
                UnregisterClassW(WINDOW_CLASS, GetModuleHandleW(nullptr));
                s_instance = nullptr;
            }

        private:
            static std::wstring Utf8ToWide(const char* text) {
                if (!text)
                    return L"Publishing";
                int length = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
                std::wstring result((size_t) length, L'\0');
                if (length > 0)
                    MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), length);
                if (!result.empty() && result.back() == L'\0')
                    result.pop_back();
                return result;
            }

            static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
                if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
                    return true;

                switch (message) {
                    case WM_SIZE:
                        if (s_instance)
                            s_instance->ResizeRenderTarget();
                        return 0;
                    case WM_CLOSE:
                        if (s_instance) {
                            if (s_instance->m_progress)
                                s_instance->m_progress->RequestCancel();
                            s_instance->m_closeRequested = true;
                        }
                        return 0;
                    case WM_DESTROY:
                        // This dialog lives inside the Animate process. Posting WM_QUIT here would
                        // shut down the host after every publish.
                        if (s_instance)
                            s_instance->m_closeRequested = true;
                        return 0;
                    default:
                        break;
                }

                return DefWindowProcW(hwnd, message, wParam, lParam);
            }

            bool CreateDevice() {
                DXGI_SWAP_CHAIN_DESC desc = {};
                desc.BufferCount = 2;
                desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.OutputWindow = m_hwnd;
                desc.SampleDesc.Count = 1;
                desc.Windowed = TRUE;
                desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                HRESULT status = D3D11CreateDeviceAndSwapChain(nullptr,
                                                               D3D_DRIVER_TYPE_HARDWARE,
                                                               nullptr,
                                                               0,
                                                               nullptr,
                                                               0,
                                                               D3D11_SDK_VERSION,
                                                               &desc,
                                                               &m_swapChain,
                                                               &m_device,
                                                               nullptr,
                                                               &m_context);
                return SUCCEEDED(status);
            }

            void ResizeRenderTarget() {
                if (!m_device || !m_swapChain)
                    return;

                if (m_renderTarget) {
                    m_renderTarget->Release();
                    m_renderTarget = nullptr;
                }

                ID3D11Texture2D* backBuffer = nullptr;
                if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
                    return;
                m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTarget);
                backBuffer->Release();

                RECT rect = {};
                GetClientRect(m_hwnd, &rect);
                D3D11_VIEWPORT viewport = {};
                viewport.Width = (float) (rect.right - rect.left);
                viewport.Height = (float) (rect.bottom - rect.top);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                m_context->RSSetViewports(1, &viewport);
            }

            HWND m_hwnd = nullptr;
            bool m_backendInitialized = false;
            ExportProgress* m_progress = nullptr;
            bool m_closeRequested = false;
            IDXGISwapChain* m_swapChain = nullptr;
            ID3D11Device* m_device = nullptr;
            ID3D11DeviceContext* m_context = nullptr;
            ID3D11RenderTargetView* m_renderTarget = nullptr;
        };

        ProgressBackendWin32* ProgressBackendWin32::s_instance = nullptr;
    }

    std::unique_ptr<IProgressBackend> CreateProgressBackend() {
        auto backend = std::make_unique<ProgressBackendWin32>();
        ProgressBackendWin32::s_instance = backend.get();
        return backend;
    }
}
