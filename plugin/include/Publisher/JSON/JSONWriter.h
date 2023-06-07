#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "JSONMovieclipWriter.h"
#include "JSONShapeWriter.h"

#include "libjson.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class JSONWriter : public SharedWriter {
			Console console;
			PIFCMCallback m_callback = nullptr;

			JSONNode m_modifiers = JSONNode(JSON_ARRAY);
			JSONNode m_movieclips = JSONNode(JSON_ARRAY);
			JSONNode m_shapes = JSONNode(JSON_ARRAY);

			fs::path outputFolder;
			fs::path outputFilepath;
			
		public:
			fs::path imageFolder;
			U_Int32 imageCount = 0;

			void Init(PIFCMCallback callback, const PublisherConfig& config) {
				m_callback = callback;
				outputFilepath = config.output;
				outputFolder = config.output.parent_path();
				imageFolder = fs::path(outputFolder) / "images";

				console.Init("JSONWriter", m_callback);

				Result res = Utils::CreateDir(imageFolder.string(), m_callback);

				if (FCM_FAILURE_CODE(res)) {
					console.log("Failed to create output directory with images: %s", imageFolder.c_str());
				}
			}

			pSharedMovieclipWriter AddMovieclip() {
				JSONMovieclipWriter* builder = new JSONMovieclipWriter();
				builder->Init(this, m_callback);

				return pSharedMovieclipWriter(builder);
			}

			void AddMovieclip(JSONNode movieclip) {
				m_movieclips.push_back(movieclip);
			}

			pSharedShapeWriter AddShape() {
				JSONShapeWriter* builder = new JSONShapeWriter();
				builder->Init(this, m_callback);

				return pSharedShapeWriter(builder);
			}

			void AddShape(JSONNode shape) {
				m_shapes.push_back(shape);
			}

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
				JSONNode modifier;
				modifier.push_back(JSONNode("id", id));
				modifier.push_back(JSONNode("type", (uint8_t)type));
				m_modifiers.push_back(modifier);
			}

			void Finalize() {
				JSONNode root;

				m_shapes.set_name("shapes");
				root.push_back(
					m_shapes
				);

				m_modifiers.set_name("modifiers");
				root.push_back(
					m_modifiers
				);

				m_movieclips.set_name("movieclips");
				root.push_back(
					m_movieclips
				);


				std::fstream file(outputFilepath, std::ios_base::trunc | std::ios_base::out);
				file << root.write_formatted();
				file.close();
			}
		};
	}
}