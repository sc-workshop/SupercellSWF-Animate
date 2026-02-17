#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/wx.h>

namespace sc::Adobe::ui {
    class ProgressBar : public wxPanel {
    public:
        ProgressBar(wxWindow* parent);

    public:
        void SetRange(int range) {
            m_range = std::max(1, range);
            Refresh();
        }

        int GetRange() const { return m_range; }

        void SetValue(int value) {
            m_value = std::clamp(value, 0, m_range);
            Refresh();
        }

        int GetValue() const { return m_value; }

    private:
        int m_value = 0;
        int m_range = 100;

        void OnPaint(wxPaintEvent&);
    };
}
