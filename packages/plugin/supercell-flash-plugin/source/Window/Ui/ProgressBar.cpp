#include "ProgressBar.h"

namespace sc::Adobe::ui {
    ProgressBar::ProgressBar(wxWindow* parent)
        : wxPanel(parent, wxID_ANY)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &ProgressBar::OnPaint, this);
    }

    void ProgressBar::OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(wxColor(0x333333));
        dc.Clear();

        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
        if (!gc)
            return;

        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

        wxSize size = GetClientSize();
        const double radius = size.y / 2.0;
        const double progress = static_cast<double>(m_value) / m_range;
        const double filledWidth = size.x * progress;

        wxGraphicsPath bgPath = gc->CreatePath();
        bgPath.AddRoundedRectangle(0, 0, size.x, size.y, radius);
        wxColour bgColor(40, 40, 40, 180);
        gc->SetBrush(wxBrush(bgColor));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->FillPath(bgPath);

        if (filledWidth > 0) {
            wxGraphicsPath fillPath = gc->CreatePath();
            fillPath.AddRoundedRectangle(0, 0, filledWidth, size.y, radius);

            wxColour topColor(90, 210, 255, 255);
            wxColour midColor(0, 120, 215, 255);
            wxColour bottomColor(0, 80, 200, 255);

            wxGraphicsGradientStops stops;
            stops.Add(topColor, 0.0);
            stops.Add(midColor, 0.25);
            stops.Add(bottomColor, 1.0);

            auto brush = gc->CreateLinearGradientBrush(0, 0, 0, size.y, stops);
            gc->SetBrush(brush);
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->FillPath(fillPath);
        }

        wxColour borderColor(50, 50, 50);
        gc->SetPen(wxPen(borderColor, 2));
        gc->StrokePath(bgPath);
    }
}