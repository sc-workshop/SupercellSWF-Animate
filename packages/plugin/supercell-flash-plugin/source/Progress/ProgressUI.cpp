#include "ProgressUI.h"

#include "Module/Localization.h"
#include "Module/Module.h"
#include "Progress/ProgressBackend.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fmt/format.h>
#include <string>
#include <utility>

namespace sc::Adobe {
    namespace {
        const ImVec4 ACCENT = ImVec4(0.08f, 0.45f, 0.90f, 1.00f);
        const ImVec4 TEXT = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        const ImVec4 MUTED = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
        const ImVec4 ERROR = ImVec4(0.95f, 0.40f, 0.40f, 1.00f);

        void SetupFont() {
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();

#if defined(_WIN32)
            const char* candidates[] = {"C:/Windows/Fonts/segoeui.ttf",
                                        "C:/Windows/Fonts/segoeuib.ttf",
                                        "C:/Windows/Fonts/arial.ttf"};
#else
            const char* candidates[] = {"/System/Library/Fonts/SFNS.ttf",
                                        "/System/Library/Fonts/Helvetica.ttc",
                                        "/System/Library/Fonts/Arial.ttf"};
#endif

            for (const char* path : candidates) {
                if (fs::exists(path)) {
                    io.Fonts->AddFontFromFileTTF(path, 18.0f);
                    return;
                }
            }

            io.Fonts->AddFontDefault();
        }

        /// @brief Dark, compact palette matching Adobe Animate panels
        void ApplyAdobeStyle() {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowPadding = ImVec2(28.0f, 24.0f);
            style.FramePadding = ImVec2(10.0f, 8.0f);
            style.ItemSpacing = ImVec2(12.0f, 10.0f);
            style.ItemInnerSpacing = ImVec2(10.0f, 6.0f);
            style.WindowRounding = 6.0f;
            style.FrameRounding = 4.0f;
            style.ScrollbarRounding = 3.0f;
            style.GrabRounding = 2.0f;
            style.WindowBorderSize = 1.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.FrameBorderSize = 1.0f;

            ImVec4* colors = style.Colors;
            colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
            colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
            colors[ImGuiCol_Border] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
            colors[ImGuiCol_Button] = ACCENT;
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.51f, 0.94f, 1.00f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.36f, 0.76f, 1.00f);
            colors[ImGuiCol_Text] = TEXT;
            colors[ImGuiCol_TextDisabled] = MUTED;
            colors[ImGuiCol_TextSelectedBg] = ImVec4(0.08f, 0.45f, 0.90f, 0.35f);
            colors[ImGuiCol_PlotHistogram] = ACCENT;
            colors[ImGuiCol_Separator] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
        }

        std::string FormatTime(double seconds) {
            if (seconds < 0.0)
                seconds = 0.0;
            int total = (int) seconds;
            int minutes = total / 60;
            int rest = total % 60;
            return fmt::format("{:02d}:{:02d}", minutes, rest);
        }

        const char* PhaseTID(ExportPhase phase) {
            switch (phase) {
                case ExportPhase::Preparing:
                    return "TID_PHASE_PREPARING";
                case ExportPhase::LoadingDocuments:
                    return "TID_PHASE_DOCUMENTS";
                case ExportPhase::ExternalFile:
                    return "TID_PHASE_EXTERNAL";
                case ExportPhase::Extracting:
                    return "TID_PHASE_EXTRACTING";
                case ExportPhase::Committing:
                    return "TID_PHASE_COMMITTING";
                case ExportPhase::Atlas:
                    return "TID_PHASE_ATLAS";
                case ExportPhase::Saving:
                    return "TID_PHASE_SAVING";
                case ExportPhase::Done:
                    return "TID_PHASE_DONE";
                case ExportPhase::Failed:
                    return "TID_PHASE_FAILED";
                case ExportPhase::Cancelled:
                    return "TID_PHASE_CANCELLED";
                default:
                    return "TID_PHASE_PREPARING";
            }
        }

        void DrawProgress(const ProgressUIConfig& config, bool& dismissed) {
            ExportProgress* progress = config.progress;
            const ProgressSnapshot snapshot = progress->Snapshot();

            auto tr = [&](const char* tid, auto&&... args) -> std::string {
                if (config.locale)
                    return config.locale->GetU8String(tid, std::forward<decltype(args)>(args)...);
                return fmt::format(fmt::runtime(std::string(tid)), std::forward<decltype(args)>(args)...);
            };

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

            constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                               ImGuiWindowFlags_NoFocusOnAppearing;

            if (!ImGui::Begin("##PublishProgress", nullptr, flags)) {
                ImGui::End();
                return;
            }

            std::string title = snapshot.documentName.empty()
                                    ? tr("TID_PROGRESS_WINDOW_TITLE")
                                    : tr("TID_PROGRESS_WINDOW_DOCUMENT", snapshot.documentName);

            ImGui::TextColored(ACCENT, "%s", title.c_str());
            ImGui::Separator();

            if (snapshot.outcome == ExportOutcome::Failure) {
                ImGui::TextColored(ERROR, "%s", tr("TID_PHASE_FAILED").c_str());
                ImGui::TextWrapped("%s", snapshot.error.c_str());
            } else {
                std::string phase = tr(PhaseTID(snapshot.phase));
                if (snapshot.cancelling && snapshot.outcome == ExportOutcome::None)
                    phase += "  -  " + tr("TID_PROGRESS_CANCELLING");
                ImGui::TextColored(ACCENT, "%s", phase.c_str());

                if (!snapshot.detail.empty())
                    ImGui::TextUnformatted(snapshot.detail.c_str());
                if (!snapshot.subdetail.empty())
                    ImGui::TextColored(MUTED, "%s", snapshot.subdetail.c_str());

                ImGui::Spacing();
                ImGui::Spacing();

                std::string overlay;
                if (snapshot.total > 0)
                    overlay = fmt::format("{}/{}", std::min(snapshot.done, snapshot.total), snapshot.total);

                float fraction =
                    snapshot.total > 0 ? std::min(1.0f, (float) snapshot.done / (float) snapshot.total) : -1.0f;
                constexpr float BAR_INSET = 12.0f;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + BAR_INSET);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 9.0f));
                ImGui::ProgressBar(fraction,
                                   ImVec2(-2.0f * BAR_INSET, 0.0f),
                                   overlay.empty() ? nullptr : overlay.c_str());
                ImGui::PopStyleVar();
                ImGui::Spacing();

                if (snapshot.documentCount > 1) {
                    ImGui::TextColored(MUTED,
                                       "%s",
                                       tr("TID_PROGRESS_DOCUMENT",
                                          std::min(snapshot.document + 1, snapshot.documentCount),
                                          snapshot.documentCount)
                                           .c_str());
                }
            }

            double eta = -1.0;
            if (snapshot.total > 0 && snapshot.done > 0 && snapshot.done < snapshot.total)
                eta = snapshot.elapsedSeconds / (double) snapshot.done * (double) (snapshot.total - snapshot.done);

            std::string stats =
                tr("TID_PROGRESS_STATS", FormatTime(snapshot.elapsedSeconds), eta >= 0.0 ? FormatTime(eta) : "--:--");
            ImGui::TextColored(MUTED, "%s", stats.c_str());

            const float buttonHeight = ImGui::GetFrameHeight();
            const float footerTop = ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y - buttonHeight;
            const float currentY = ImGui::GetCursorPosY();
            if (footerTop > currentY)
                ImGui::Dummy(ImVec2(0.0f, footerTop - currentY));

            std::string buttonLabel =
                snapshot.outcome == ExportOutcome::Failure ? tr("TID_PROGRESS_CLOSE") : tr("TID_PROGRESS_CANCEL");
            bool disableButton = snapshot.outcome != ExportOutcome::None ? snapshot.outcome != ExportOutcome::Failure
                                                                         : snapshot.cancelling;
            if (disableButton)
                ImGui::BeginDisabled();
            if (ImGui::Button(buttonLabel.c_str(), ImVec2(130.0f, 0.0f))) {
                if (snapshot.outcome == ExportOutcome::Failure)
                    dismissed = true;
                else
                    progress->RequestCancel();
            }
            if (disableButton)
                ImGui::EndDisabled();

            ImGui::End();
        }
    }

    bool RunProgressUI(const ProgressUIConfig& config) {
        if (!config.progress)
            return false;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DisplaySize = ImVec2(PROGRESS_WINDOW_WIDTH, PROGRESS_WINDOW_HEIGHT);
        SetupFont();
        ApplyAdobeStyle();

        std::string title =
            config.locale ? config.locale->GetU8String("TID_PROGRESS_WINDOW_TITLE") : std::string("Publishing");
        auto backend = CreateProgressBackend();
        if (!backend || !backend->Create(title.c_str(), config.progress)) {
            ImGui::DestroyContext();
            return false;
        }

        bool dismissed = false;
        while (backend->IsOpen()) {
            ProgressSnapshot snapshot = config.progress->Snapshot();
            if (snapshot.outcome == ExportOutcome::Failure) {
                if (dismissed)
                    break;
            } else if (config.progress->IsFinished()) {
                break;
            }

            backend->BeginFrame();
            DrawProgress(config, dismissed);
            backend->EndFrame();

            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                if (snapshot.outcome == ExportOutcome::Failure)
                    dismissed = true;
                else if (!config.progress->IsFinished())
                    config.progress->RequestCancel();
            }
        }

        backend->Destroy();
        ImGui::DestroyContext();
        return true;
    }
}
