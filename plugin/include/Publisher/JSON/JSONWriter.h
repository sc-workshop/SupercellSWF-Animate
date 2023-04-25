#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "JSONMovieclipWriter.h"
#include "JSONShapeWriter.h"

#include "libjson.h"

using namespace FCM;
using namespace Adobe;

namespace sc {
	namespace Adobe {
		class JSONWriter : public SharedWriter {
			Console console;
			PIFCMCallback m_callback = nullptr;

			JSONNode* m_movieclips;
			JSONNode m_shapes = JSONNode(JSON_ARRAY);
			
		public:
			fs::path imageFolder;
			U_Int32 imageCount = 0;

			JSONWriter() {
				m_movieclips = new JSONNode(JSON_ARRAY);
				m_movieclips->set_name("MovieClips");

			}
			~JSONWriter() {
				delete m_movieclips;
			}

			Result Init(PIFCMCallback callback, fs::path folder) {
				if (!callback) {
					return FCM_EXPORT_FAILED;
				}
				m_callback = callback;
				baseFolder = folder;

				console.Init("JSONWriter", m_callback);

				imageFolder = baseFolder / "images";

				Result res = Utils::CreateDir(imageFolder.string(), m_callback);

				if (FCM_FAILURE_CODE(res)) {
					console.log("Failed to create output directory with images: %s", imageFolder.c_str());
				}

				return FCM_SUCCESS;
			}

			SharedMovieclipWriter* AddMovieclip() {
				JSONMovieclipWriter* builder = new JSONMovieclipWriter();
				builder->Init(this, m_callback);

				return builder;
			}

			void AddMovieclip(JSONNode movieclip) {
				m_movieclips->push_back(movieclip);
			}

			SharedShapeWriter* AddShape() {
				JSONShapeWriter* builder = new JSONShapeWriter();
				builder->Init(this, m_callback);

				return builder;
			}

			void AddShape(JSONNode shape) {
				m_shapes.push_back(shape);
			}

			void Finalize(std::string filename) {
				JSONNode root;

				root.push_back(
					*m_movieclips
				);

				m_shapes.set_name("Shapes");
				root.push_back(
					m_shapes
				);

				std::fstream file;
				Utils::OpenFStream((baseFolder / filename).string(), file, std::ios_base::trunc | std::ios_base::out, m_callback);

				file << root.write_formatted();
				file.close();
			}
		};
	}
}