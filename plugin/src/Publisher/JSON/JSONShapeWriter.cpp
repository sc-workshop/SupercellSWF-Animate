#include "Publisher/JSON/JSONShapeWriter.h"

#include "Publisher/JSON/JSONWriter.h"
#include "Utils.h"

namespace sc {
	namespace Adobe {
		void JSONShapeWriter::Init(JSONWriter* writer) {
			if (writer) {
				m_writer = writer;
			}
			else {
				throw exception("Failed to get writer");
			}
		}

		void JSONShapeWriter::AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) {
			uint32_t imageIndex = m_writer->imageCount;
			m_writer->imageCount++;

			std::string bitmapBasename = std::to_string(imageIndex) + ".png";
			fs::path bitmapOutputPath = m_writer->imageFolder / bitmapBasename;

			cv::imwrite(bitmapOutputPath.string(), image);

			m_bitmaps.push_back({
				{"type", "graphic"},
				{"path", bitmapBasename},
				{"matrix", Utils::ToString(matrix)}
				});
		}

		void JSONShapeWriter::AddFilledShape(const FilledShape& shape, bool isNineSlice) {
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

					result.push_back({
						{"type", (uint8_t)region.type},
						{"points", points},
						{"holes", holes}
						});
				}

				return result;
			};

			ordered_json shapeData;
			shapeData["type"] = "shape";
			shapeData["fills"] = processRegions(shape.fill);
			shapeData["stroke"] = processRegions(shape.stroke);

			m_bitmaps.push_back(shapeData);
		}

		void JSONShapeWriter::Finalize(uint16_t id) {
			ordered_json root = ordered_json::object({
				{"id", id},
				{"bitmaps", m_bitmaps}
				});

			m_writer->AddShape(root);
		}
	}
}