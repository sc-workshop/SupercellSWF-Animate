#include "ResourcePublisher/Writer/SWF/Writer.h"
#include "ResourcePublisher/Writer/SWF/ShapeWriter.h"
#include "Module/Context.h"
#include "Utils.h"

#include "opencv2/opencv.hpp"
using namespace sc;

#include "CDT.h"

namespace sc {
	namespace Adobe {
		void SCShapeWriter::Init(Context&, SymbolContext&) {
		}

		void SCShapeWriter::AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) {
			Sprite sprite;
			sprite.image = image.clone();
			sprite.matrix = matrix;

			m_sprites.push_back(sprite);
		}

		void SCShapeWriter::AddFilledShape(const FilledShape& shape) {
			for (const FilledShapeRegion& region : shape.fill) {
				AddFilledShapeRegion(region);
			}

			for (const FilledShapeRegion& region : shape.stroke) {
				AddFilledShapeRegion(region);
			}
		}

		void SCShapeWriter::AddRasterizedSolidColorRegion(FilledShapePath* contour, std::vector<FilledShapePath*> holes, cv::Scalar color)
		{
			if (!contour) return;
			// Region bounding box
			float minX = 0;
			float minY = 0;
			float maxX = 0;
			float maxY = 0;

			for (const Point2D& point : contour->points) {
				if (point.x > maxX) maxX = point.x;
				if (point.y > maxY) maxY = point.y;

				if (point.x < minX) minX = point.x;
				if (point.y < minY) minY = point.y;
			}

			Sprite sprite;
			sprite.image = cv::Mat(abs(maxY - minY), abs(maxX - minX), CV_8UC4, cv::Scalar(0, 0, 0, 0));
			sprite.matrix = { 1, 0, 0, 1, (float)minX, (float)minY };

			// Contour drawing
			std::vector<cv::Point> fillContour;
			for (const Point2D& point : contour->points) {
				fillContour.push_back(cv::Point(point.x - minX, point.y - minY));
			}

			cv::fillPoly(sprite.image, fillContour, color);

			// Hole drawing
			for (const auto& holePath : holes) {
				std::vector<cv::Point> hole;

				for (const Point2D& point : holePath->points) {
					hole.push_back(cv::Point(point.x - minX, point.y - minY));
				}

				cv::fillPoly(sprite.image, hole, cv::Scalar(0, 0, 0, 0));
			}

			m_sprites.push_back(sprite);
		}

		void SCShapeWriter::AddSolidColorRegion(FilledShapePath* contour, std::vector<FilledShapePath*> holes, cv::Scalar color)
		{
			if (!contour) return;

			Sprite sprite;
			sprite.image = cv::Mat(cv::Size(1, 1), CV_8UC4, color);
			sprite.matrix = { 1, 0, 0, 1, 0, 0 };

			// If region is "simple"
			if (holes.size() == 0 &&
				contour->points.size() <= 8 && contour->points.size() >= 3)
			{
				sprite.contour = contour->points;
				m_sprites.push_back(sprite);
			}
			else {
				CDT::Triangulation<float> cdt;

				std::vector<CDT::V2d<float>> vertices;
				std::vector<CDT::Edge> edges;

				for (const Point2D& point : contour->points) {
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

				// Holes
				for (FilledShapePath* hole : holes) {
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

					m_sprites.push_back(triangleSprite);
				}
			}
		}

		void SCShapeWriter::AddFilledShapeRegion(const FilledShapeRegion& region) {
			if (!region.contour) return;

			switch (region.type)
			{
			case FilledShapeType::SolidColor:
			{
				cv::Scalar solidColor = cv::Scalar(
					(region.solidColor >> 16) & 0xFF, // R
					(region.solidColor >> 8) & 0xFF, // G
					region.solidColor & 0xFF, // B

					(region.solidColor >> 24) & 0xFF
				);

				AddSolidColorRegion(region.contour.get(),
					([region]() {
						std::vector<FilledShapePath*> holes;
						std::transform(region.holes.cbegin(), region.holes.cend(), std::back_inserter(holes), [](auto& ptr) { return ptr.get(); });
						return holes;
						})(),
							solidColor);
			}
			break;
			default:
				break;
			}
		}

		void SCShapeWriter::Finalize(uint16_t id) {
			pShape shape = pShape(new Shape());
			shape->id(id);
			shape->commands.resize(m_sprites.size());
			m_writer->sprites.insert(m_writer->sprites.end(), m_sprites.begin(), m_sprites.end());

			m_writer->swf.shapes.push_back(shape);
		}
	}
}