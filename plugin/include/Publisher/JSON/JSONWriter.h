#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "JSONMovieclipWriter.h"
#include "JSONShapeWriter.h"

#include "JSON.hpp"

using namespace FCM;
using namespace std;
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class JSONWriter : public SharedWriter {
			json m_modifiers = json::array();
			json m_movieclips = json::array();
			json m_shapes = json::array();

		public:
			using SharedWriter::SharedWriter;

			fs::path imageFolder;
			U_Int32 imageCount = 0;

			void Init() {
				imageFolder = m_context.config.output.parent_path() / "images";
				if (fs::exists(imageFolder)) {
					fs::remove_all(imageFolder);
				}
				fs::create_directory(imageFolder);
			}

			pSharedMovieclipWriter AddMovieclip() {
				JSONMovieclipWriter* builder = new JSONMovieclipWriter();
				builder->Init(this);

				return pSharedMovieclipWriter(builder);
			}

			void AddMovieclip(json movieclip) {
				m_movieclips.push_back(movieclip);
			}

			pSharedShapeWriter AddShape() {
				JSONShapeWriter* builder = new JSONShapeWriter();
				builder->Init(this);

				return pSharedShapeWriter(builder);
			}

			void AddShape(json shape) {
				m_shapes.push_back(shape);
			}

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
				m_modifiers.push_back({
					{"id", id},
					{"type", (uint8_t)type}
				});
			}

			void Finalize() {
				json root = {
					{"shapes", m_shapes },
					{"modifiers", m_modifiers},
					{"movieclips", m_movieclips}
				};

				ofstream file(m_context.config.output);
				file << setw(4) << root << endl;
				file.close();
			}
		};
	}
}