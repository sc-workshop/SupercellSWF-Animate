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

		void ShapeWriter::AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) {
			Sprite sprite;
			sprite.image = image.clone();
			sprite.matrix = matrix;

			m_writer->sprites.push_back(sprite);
			m_bitmapCount++;
		}

		void ShapeWriter::AddFilledShape(const FilledShape& shape) {
			for (const FilledShapeRegion& region : shape.fill) {
				AddFilledShapeRegion(region);
			}
		}

		void ShapeWriter::AddFilledShapeRegion(const FilledShapeRegion& region) {
			if (!region.contour) return;

			if (region.type != FilledShapeType::SolidColor) return;

			// If region is "simple"
			if (region.holes.size() == 0 &&
				region.contour->points.size() <= 8 && region.contour->points.size() <= 3 &&
				region.type == FilledShapeType::SolidColor) {

				Sprite sprite;
				sprite.image = cv::Mat(cv::Size(1, 1), CV_8UC4, cv::Scalar(
					(region.solidColor >> 16) & 0xFF,
					(region.solidColor >> 8) & 0xFF,
					region.solidColor & 0xFF,

					(region.solidColor >> 24) & 0xFF
				));
				sprite.contour = region.contour->points;
				if (sprite.contour.size() == 3) {
					sprite.contour.push_back(sprite.contour[2]);
				}
				sprite.matrix = { 1, 0, 0, 1, 0, 0 };

				m_bitmapCount++;
				m_writer->sprites.push_back(sprite);
			}

		}

		void ShapeWriter::Finalize(uint16_t id) {
			pShape shape = pShape(new Shape());
			shape->id(id);
			shape->commands.resize(m_bitmapCount);

			m_writer->m_swf.shapes.push_back(shape);
		}
	}
}