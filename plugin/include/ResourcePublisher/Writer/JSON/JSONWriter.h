#pragma once

#include "ResourcePublisher/Writer/Shared/SharedWriter.h"

#include "JSONMovieclipWriter.h"
#include "JSONShapeWriter.h"

#include "json.hpp"
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class JSONWriter : public SharedWriter {
			Context* m_context = nullptr;

			json m_shapes = json::array();
			json m_textfields = json::array();
			json m_modifiers = json::array();
			json m_movieclips = json::array();

		public:
			std::u16string saveName;
			fs::path saveFolder;
			fs::path imageFolder;
			uint32_t imageCount = 0;

			void Init(Context& context);

			pSharedMovieclipWriter AddMovieclip();

			void AddMovieclip(ordered_json& movieclip);

			pSharedShapeWriter AddShape();

			void AddShape(ordered_json& shape);

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type);

			void AddTextField(uint16_t id, TextFieldInfo field);

			void AddExportName(uint16_t id, std::u16string name);

			void Finalize();
		};
	}
}