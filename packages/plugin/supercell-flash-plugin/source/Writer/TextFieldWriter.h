#pragma once

#include "AnimateCore.h"
#include "AnimatePublisher.h"
#include "flash/flash.h"

namespace sc::Adobe {
    class SCWriter;

    class SCTextFieldWriter : public Animate::Publisher::SharedTextFieldWriter {
    public:
        SCTextFieldWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol);

        virtual void Initialize(const Animate::Publisher::TextElement& textfield) override;
        virtual void AddParagraph(const Animate::Publisher::TextParagraph& paragraph) override;
        void SetTextRun(const Animate::Publisher::TextRun& textRun);
        virtual void SetGlowFilter(const Animate::Publisher::GlowFilter& filter) override;

        virtual bool Finalize(Animate::Publisher::ResourceReference reference, bool required, bool new_symbol) override;

    protected:
        virtual std::size_t GenerateHash() const override;

    private:
        SCWriter& m_writer;

        bool m_has_paragraph = false;
        flash::TextField m_object;
    };
}
