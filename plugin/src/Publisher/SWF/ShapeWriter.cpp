#include "Publisher/SWF/ShapeWriter.h"
#include "Publisher/SWF/Writer.h"
#include "Utils.h"

#include "opencv2/opencv.hpp"
using namespace sc;

namespace sc {
	namespace Adobe {
		void ShapeWriter::Init(Writer* writer) {
			if (writer) {
				m_writer = writer;
			}
			else {
				throw exception("Failed to get writer");
			}			
		}

		void ShapeWriter::AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix) {
			Sprite sprite;
			sprite.image = image.clone();
			sprite.matrix = matrix;

			m_writer->sprites.push_back(sprite);
			m_bitmapCount++;
		}

		void ShapeWriter::Finalize(uint16_t id) {
			pShape shape = pShape(new Shape());
			shape->id(id);
			shape->commands.resize(m_bitmapCount);

			m_writer->m_swf.shapes.push_back(shape);
		}
	}
}