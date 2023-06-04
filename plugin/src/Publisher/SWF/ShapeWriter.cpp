#include "Publisher/SWF/ShapeWriter.h"
#include "Publisher/SWF/Writer.h"
#include "Utils.h"

#include "opencv2/opencv.hpp"
using namespace sc;

namespace sc {
	namespace Adobe {
		void ShapeWriter::Init(Writer* writer, PIFCMCallback callback) {
			m_writer = writer;
			m_callback = callback;
		}

		void ShapeWriter::AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix) {
			Sprite sprite;
			sprite.image = image.clone();
			sprite.matrix = matrix;

			m_writer->sprites.push_back(sprite);
			m_bitmapCount++;
		}

		void ShapeWriter::Finalize(U_Int16 id) {
			pShape shape = pShape(new Shape());
			shape->id(id);
			shape->commands.resize(m_bitmapCount);

			m_writer->swf.shapes.push_back(shape);
		}
	}
}