#include "Publisher/SWF/ShapeWriter.h"
#include "Publisher/SWF/Writer.h"
#include "Utils.h"

#include "opencv2/opencv.hpp"
using namespace sc;

#include "CDT.h"

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

		void ShapeWriter::AddFilledShape(const FilledShape& shape, bool isNineSlice) {
			for (const FilledShapeRegion& region : shape.fill) {
				AddFilledShapeRegion(region, isNineSlice);
			}

			for (const FilledShapeRegion& region : shape.stroke) {
				AddFilledShapeRegion(region, isNineSlice);
			}
		}

		void ShapeWriter::AddFilledShapeRegion(const FilledShapeRegion& region, bool isNineSlice) {
			if (!region.contour) return;

			bool rasterizeRegion = isNineSlice || region.type != FilledShapeType::SolidColor;

			cv::Scalar solidColor = cv::Scalar(
				(region.solidColor >> 16) & 0xFF, // R
				(region.solidColor >> 8) & 0xFF, // G
				region.solidColor & 0xFF, // B

				(region.solidColor >> 24) & 0xFF
			);

			if (region.type == FilledShapeType::SolidColor && solidColor[3] == 0) return;

			Sprite sprite;
			if (rasterizeRegion) {
				// Region bounding box
				float minX = 0;
				float minY = 0;
				float maxX = 0;
				float maxY = 0;

				for (const Point2D& point : region.contour->points) {
					if (point.x > maxX) maxX = point.x;
					if (point.y > maxY) maxY = point.y;

					if (point.x < minX) minX = point.x;
					if (point.y < minY) minY = point.y;
				}

				sprite.image = cv::Mat(abs(maxY - minY), abs(maxX - minX), CV_8UC4, cv::Scalar(0, 0, 0, 0));
				sprite.matrix = { 1, 0, 0, 1, (float)minX, (float)minY };

				// Contour drawing
				std::vector<cv::Point> contour;
				for (const Point2D& point : region.contour->points) {
					contour.push_back(cv::Point(point.x - minX, point.y - minY));
				}

				switch (region.type)
				{
				case FilledShapeType::SolidColor:
					cv::fillPoly(sprite.image, contour, solidColor);
					break;
				default:
					break;
				}

				// Hole drawing
				for (const auto& holePath : region.holes) {
					std::vector<cv::Point> hole;

					for (const Point2D& point : holePath->points) {
						hole.push_back(cv::Point(point.x - minX, point.y - minY));
					}

					cv::fillPoly(sprite.image, hole, cv::Scalar(0, 0, 0, 0));
				}

				m_bitmapCount++;
				m_writer->sprites.push_back(sprite);
			}
			else {
				sprite.image = cv::Mat(cv::Size(1, 1), CV_8UC4, solidColor);
				sprite.matrix = { 1, 0, 0, 1, 0, 0 };

				// If region is "simple"
				if (region.holes.size() == 0 &&
					region.contour->points.size() <= 8 && region.contour->points.size() >= 3 &&
					region.type == FilledShapeType::SolidColor) {
					sprite.contour = region.contour->points;

					m_bitmapCount++;
					m_writer->sprites.push_back(sprite);
				}
				else {
					CDT::Triangulation<float> cdt;

					std::vector<CDT::V2d<float>> vertices;
					std::vector<CDT::Edge> edges;

					for (const Point2D& point : region.contour->points) {
						vertices.push_back({ point.x, point.y });
					}

					// Contour
					for (uint32_t i = 0; vertices.size() > i; i++) {
						uint32_t secondIndex = i + 1;
						if (secondIndex >= vertices.size()) {
							secondIndex = 0;
						}
						edges.push_back(CDT::Edge(i, secondIndex));
					}

					//// Holes
					for (auto hole : region.holes) {
						if (!hole) continue;

						for (uint32_t i = 0; hole->points.size() > i; i++) {
							uint32_t secondIndex = vertices.size() + i + 1;
							if (secondIndex >= hole->points.size() + vertices.size()) {
								secondIndex = vertices.size();
							}
							edges.push_back(CDT::Edge(vertices.size() + i, secondIndex));
						}

						for (const Point2D& point : hole->points) {
							vertices.push_back({ point.x, point.y });
						}
					}

					CDT::RemoveDuplicatesAndRemapEdges(vertices, edges);

					cdt.insertVertices(vertices);
					cdt.insertEdges(edges);

					cdt.eraseOuterTrianglesAndHoles();

					for (const CDT::Triangle& triangle : cdt.triangles) {
						Sprite triangleSprite = Sprite(sprite);

						for (uint8_t i = 0; 3 > i; i++) {
							triangleSprite.contour.push_back(Point2D{ cdt.vertices[triangle.vertices[i]].x, cdt.vertices[triangle.vertices[i]].y });
						}

						m_bitmapCount++;
						m_writer->sprites.push_back(triangleSprite);
					}
				}
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