#include "ResourcePublisher/Writer/JSON/JSONWriter.h"
#include "ResourcePublisher/Writer/JSON/JSONShapeWriter.h"
#include "Module/Context.h"

namespace sc {
	namespace Adobe {
		void JSONShapeWriter::Init(Context&, SymbolBehaviorInfo&) {};

		void JSONShapeWriter::AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) {
			uint32_t imageIndex = m_writer->imageCount;
			m_writer->imageCount++;

			std::string bitmapBasename = std::to_string(imageIndex) + ".png";
			fs::path bitmapOutputPath = m_writer->imageFolder / bitmapBasename;

			cv::imwrite(bitmapOutputPath.string(), image);

			m_bitmaps.push_back(ordered_json(
				{
					{"type", "graphic"},
					{"path", bitmapBasename},
					{"matrix", Utils::ToString(matrix)}
				}
			));
		}

		void JSONShapeWriter::AddFilledShape(const FilledShape& shape) {
			std::function processRegions = [](std::vector<FilledShapeRegion> regions) {
				json result = json::array();

				for (const FilledShapeRegion& region : regions) {
					json points = json::array();
					json holes = json::array();

					if (region.contour) {
						for (const Point2D& point : region.contour->points) {
							points.push_back(json::array({ point.x, point.y }));
						}
					}

					for (auto hole : region.holes) {
						if (hole) {
							json holePoints = json::array();
							for (const Point2D& point : hole->points) {
								holePoints.push_back(json::array({ point.x, point.y }));
							}
							holes.push_back(holePoints);
						}
					}

					result.push_back(ordered_json(
						{
							{"type", (uint8_t)region.type},
							{"points", points},
							{"holes", holes}
						}
					)
					);
				}

				return result;
			};

			m_bitmaps.push_back(ordered_json(
				{
					{"type", "fill"},
					{"fills", processRegions(shape.fill)},
					{"stroke", processRegions(shape.stroke)}
				}
			));
		}

		void JSONShapeWriter::Finalize(uint16_t id) {
			m_writer->AddShape(ordered_json::object(
				{
					{"id", id},
					{"bitmaps", m_bitmaps}
				}
			));
		}
	}
}