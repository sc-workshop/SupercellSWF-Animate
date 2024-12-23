#pragma once

#include "AnimateCore.h"
#include "AnimatePublisher.h"

#include "flash/flash.h"

namespace sc::Adobe {
	class SCWriter;

	class SCTextFieldWriter : public Animate::Publisher::SharedTextFieldWriter
	{
	public:
		SCTextFieldWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol);

		virtual void Initialize(const Animate::Publisher::TextElement& textfield);
		virtual void AddParagraph(const Animate::Publisher::TextParagraph& paragraph);
		void SetTextRun(const Animate::Publisher::TextRun& textRun);
		virtual void SetGlowFilter(const Animate::Publisher::GlowFilter& filter) override;

		virtual bool Finalize(uint16_t id, bool required);

	private:
		SCWriter& m_writer;

		bool m_has_paragraph = false;
		flash::TextField m_object;
	};
}