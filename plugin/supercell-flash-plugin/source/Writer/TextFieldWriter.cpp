#include "Writer.h"
#include "TextFieldWriter.h"
#include "Module/Module.h"

#include "core/hashing/ncrypto/xxhash.h"

using namespace Animate::Publisher;

namespace sc::Adobe {
	SCTextFieldWriter::SCTextFieldWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol) :
		Animate::Publisher::SharedTextFieldWriter(symbol),
		m_writer(writer)
	{
	};

	void SCTextFieldWriter::Initialize(const Animate::Publisher::TextElement& textfield)
	{
		using namespace Animate::DOM;

		m_object.left = (int16_t)floor(textfield.bound.topLeft.x);
		m_object.top = (int16_t)floor(textfield.bound.topLeft.y);
		
		m_object.right = (int16_t)floor(textfield.bound.bottomRight.x);
		m_object.bottom = (int16_t)floor(textfield.bound.bottomRight.y);
		m_object.text = flash::SWFString(FCM::Locale::ToUtf8(textfield.text));

		m_object.is_multiline =
			textfield.lineMode == FrameElement::LineMode::LINE_MODE_SINGLE ? false : true;

		if (textfield.renderingMode.aaMode == FrameElement::AAMode::ANTI_ALIAS_MODE_DEVICE) {
			m_object.use_device_font = true;
		}
	}

	void SCTextFieldWriter::AddParagraph(const TextParagraph& paragraph)
	{
		using namespace Animate::DOM;
		SCPlugin& context = SCPlugin::Instance();
		
		if (m_has_paragraph)
		{
			context.Trace(
				"TextField in symbol \"%ls\" frame %d, has more than one Paragraph. Skip...",
				m_symbol.name.c_str(),
				m_symbol.current_frame_index + 1
			);

			return;
		}

		switch (paragraph.style.alignment)
		{
		case FrameElement::AlignMode::ALIGN_MODE_CENTER:
			m_object.font_horizontal_align = flash::TextField::Align::Center;
			break;
		case FrameElement::AlignMode::ALIGN_MODE_JUSTIFY:
			m_object.font_horizontal_align = flash::TextField::Align::Justify;
			break;
		case FrameElement::AlignMode::ALIGN_MODE_LEFT:
			m_object.font_horizontal_align = flash::TextField::Align::Left;
			break;
		case FrameElement::AlignMode::ALIGN_MODE_RIGHT:
			m_object.font_horizontal_align = flash::TextField::Align::Right;
			break;
		default:
			break;
		}

		if (paragraph.style.lineSpacing > 0)
		{
			m_object.font_vertical_align = flash::TextField::Align::Center;
		}
		
		if (paragraph.textRuns.size() > 1)
		{
			context.Trace(
				"TextField in symbol \"%ls\" frame %d, has more than one TextRun. Using the first text run...",
				m_symbol.name.c_str(),
				m_symbol.current_frame_index + 1
			);
		}

		if (!paragraph.textRuns.empty())
		{
			SetTextRun(paragraph.textRuns[0]);
		}
	}

	void SCTextFieldWriter::SetTextRun(const TextRun& textRun)
	{
		using namespace Animate::DOM;
		const SCConfig& config = SCPlugin::Publisher::ActiveConfig();

		m_object.font_name = flash::SWFString(FCM::Locale::ToUtf8(textRun.fontName));
		m_object.font_color = *(uint32_t*)&textRun.fontColor;
		m_object.font_size = (uint8_t)textRun.fontSize;

		if (textRun.fontStyle != FrameElement::REGULAR_STYLE_STR) {
			if (textRun.fontStyle != FrameElement::ITALIC_STYLE_STR) {
				m_object.is_italic = true;
			}
			else if (textRun.fontStyle != FrameElement::BOLD_STYLE_STR) {
				m_object.is_bold = true;
			}
			else if (textRun.fontStyle != FrameElement::BOLD_ITALIC_STYLE_STR) {
				m_object.is_italic = true;
				m_object.is_bold = true;
			}
		}

		if (config.backwardCompatibility) return;
		m_object.auto_kern = textRun.autoKern == FCM::Boolean(true);
	}

	void SCTextFieldWriter::SetGlowFilter(const GlowFilter& filter)
	{
		m_object.is_outlined = true;
		m_object.outline_color = (static_cast<uint32_t>(filter.color.red) << 24) |
			(static_cast<uint32_t>(filter.color.green) << 16) |
			(static_cast<uint32_t>(filter.color.blue) << 8) |
			static_cast<uint32_t>(filter.color.alpha);
	}

	std::size_t SCTextFieldWriter::GenerateHash() const
	{
		wk::hash::XxHash code;

		code.update(m_object.text);
		code.update(m_object.font_name);
		code.update(m_object.font_color);
		code.update(m_object.font_horizontal_align);
		code.update(m_object.font_vertical_align);
		code.update(m_object.unknown_align6);
		code.update(m_object.unknown_align7);

		code.update(m_object.left);
		code.update(m_object.top);
		code.update(m_object.right);
		code.update(m_object.bottom);

		code.update(m_object.is_bold);
		code.update(m_object.is_italic);
		code.update(m_object.is_multiline);
		code.update(m_object.is_outlined);
		code.update(m_object.unknown_flag3);

		code.update(m_object.outline_color);
		code.update(m_object.use_device_font);
		code.update(m_object.auto_kern);
		code.update(m_object.bend_angle);

		code.update(m_object.unknown_flag);
		code.update(m_object.unknown_short);
		code.update(m_object.unknown_short2);

		code.update(m_object.typography_file);

		return code.digest();
	}

	bool SCTextFieldWriter::Finalize(uint16_t id, bool /*required*/)
	{
		m_object.id = id;
		m_writer.swf.textfields.push_back(m_object);
		return true;
	};

}