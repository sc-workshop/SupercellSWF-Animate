#include "ResourcePublisher/Writer/Writer.h"
#include "ResourcePublisher/Writer/ShapeWriter.h"
#include "Module/PluginContext.h"

#include "opencv2/opencv.hpp"
using namespace sc;

#include "CDT.h"

namespace sc {
	namespace Adobe {
		void SCShapeWriter::AddGraphic(const SpriteElement& item, const DOM::Utils::MATRIX2D& matrix) {
			Ref<cv::Mat> image = m_writer.GetBitmap(item);

			m_group.AddItem<SpriteItem>(image, matrix);
		}

		void SCShapeWriter::AddFilledElement(const FilledElement& shape) {
			for (const FilledElementRegion& region : shape.fill) {
				AddFilledShapeRegion(region);
			}

			for (const FilledElementRegion& region : shape.stroke) {
				AddFilledShapeRegion(region);
			}
		}

		void SCShapeWriter::AddTriangulatedRegion(
			const FilledElementPath& contour,
			const std::vector<FilledElementPath>& holes,
			const DOM::Utils::COLOR& color)
		{
			CDT::Triangulation<float> cdt;

			std::vector<CDT::V2d<float>> vertices;
			std::vector<CDT::Edge> edges;

			for (const Point2D& point : contour.points) {
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
			for (const FilledElementPath& hole : holes) {
				for (uint32_t i = 0; hole.points.size() > i; i++) {
					uint32_t secondIndex = vertices.size() + i + 1;
					if (secondIndex >= hole.points.size() + vertices.size()) {
						secondIndex = vertices.size();
					}
					edges.push_back(CDT::Edge(vertices.size() + i, secondIndex));
				}

				for (const Point2D& point : hole.points) {
					vertices.push_back({ point.x, point.y });
				}
			}

			CDT::RemoveDuplicatesAndRemapEdges(vertices, edges);

			cdt.insertVertices(vertices);
			cdt.insertEdges(edges);

			cdt.eraseOuterTrianglesAndHoles();

			std::vector<FilledItemContour> contours;

			for (const CDT::Triangle& triangle : cdt.triangles) {
				auto point1 = cdt.vertices[triangle.vertices[0]];
				auto point2 = cdt.vertices[triangle.vertices[1]];
				auto point3 = cdt.vertices[triangle.vertices[2]];

				std::vector<Point2D> triangle_shape(
					{
						{point1.x, point1.y},
						{point2.x, point2.y},
						{point3.x, point3.y},
						{point3.x, point3.y},
					}
				);

				contours.emplace_back(triangle_shape);
			}

			m_group.AddItem<FilledItem>(contours, color);
		}

		void SCShapeWriter::AddRasterizedRegion(
			const FilledElementRegion& region,
			cv::Mat& canvas,
			Point<int32_t> offset
		)
		{
			cv::Size canvas_size = canvas.size();

			// Creating fill mask
			cv::Mat canvas_mask(canvas_size, CV_8UC1, cv::Scalar(0x00));

			const int contour_shift = 8;

			// Contour
			{
				std::vector<cv::Point> points;
				for (const Point2D& point : region.contour.points)
				{
					points.emplace_back(
						(point.x - offset.x) * pow(2, contour_shift),
						(point.y - offset.y) * pow(2, contour_shift)
					);
				}

				cv::fillConvexPoly(canvas_mask, points, cv::Scalar(0xFF), cv::LINE_AA, contour_shift);
#ifdef CV_DEBUG
				cv::imshow("Contour Mask", canvas_mask);
				cv::waitKey(0);
#endif
			}

			{
				for (const FilledElementPath& path : region.holes)
				{
					std::vector<cv::Point> path_points;
					for (const Point2D& point : path.points)
					{
						path_points.emplace_back(
							(point.x - offset.x) * pow(2, contour_shift),
							(point.y - offset.y) * pow(2, contour_shift)
						);
					}

					cv::fillConvexPoly(canvas_mask, path_points, cv::Scalar(0x00), cv::LINE_AA, contour_shift);
				}

#ifdef CV_DEBUG
				cv::imshow("Contour Holes", canvas_mask);
				cv::waitKey(0);
#endif
			}

			cv::Mat filling_image(canvas.size(), CV_8UC4, cv::Scalar(0x00000000));

			// Filling
			switch (region.type)
			{
			case FilledElementRegion::ShapeType::SolidColor:
			{
				const cv::Scalar color(
					region.solidColor.blue,
					region.solidColor.green,
					region.solidColor.red,
					region.solidColor.alpha
				);

				filling_image.setTo(color);
			}
			break;
			default:
				break;
			}

			for (int h = 0; canvas_size.height > h; h++)
			{
				for (int w = 0; canvas_size.width > w; w++)
				{
					cv::Vec4b& origin = canvas.at<cv::Vec4b>(h, w);
					cv::Vec4b& destination = filling_image.at<cv::Vec4b>(h, w);
					uchar& mask_alpha = canvas_mask.at<uchar>(h, w);

					if (destination[3] == 0) continue;

					destination[3] = (uchar)std::clamp(destination[3], 0ui8, mask_alpha);

					float alpha_factor = destination[3] / 0xFF;

					origin[0] = (origin[0] * (255 - destination[3]) + destination[0] * destination[3]) / 255;
					origin[1] = (origin[1] * (255 - destination[3]) + destination[1] * destination[3]) / 255;
					origin[2] = (origin[2] * (255 - destination[3]) + destination[2] * destination[3]) / 255;

					origin[3] = (uchar)std::clamp(destination[3] + origin[3], 0, 0xFF);
				}
			}

#ifdef CV_DEBUG
			cv::imshow("Canvas Fill", canvas);
			cv::waitKey(0);
#endif
		}

		void SCShapeWriter::AddRasterizedRegion(
			const FilledElementRegion& region
		)
		{
			const DOM::Utils::RECT region_bound = region.Bound();
			Point<int32_t> image_position_offset(region_bound.bottomRight.x, region_bound.bottomRight.y);
			cv::Size image_size(
				ceil(region_bound.topLeft.x - image_position_offset.x),
				ceil(region_bound.topLeft.y - image_position_offset.y)
			);

			cv::Mat canvas(image_size, CV_8UC4, cv::Scalar(0x00000000));

			AddRasterizedRegion(region, canvas, image_position_offset);

			const DOM::Utils::MATRIX2D transform = {
				1.0f,
				0.0f,
				0.0f,
				1.0f,
				(FCM::Float)image_position_offset.x,
				(FCM::Float)image_position_offset.y
			};

			// Adding to group
			m_group.AddItem<SpriteItem>(CreateRef<cv::Mat>(canvas), transform);
		}

		bool SCShapeWriter::IsValidFilledShapeRegion(const FilledElementRegion& region)
		{
			if (region.type == FilledElementRegion::ShapeType::SolidColor)
			{
				// Skip all regions with zero mask_alpha
				if (region.solidColor.alpha <= 0)
				{
					return false;
				}
			}

			return true;
		}

		void SCShapeWriter::AddFilledShapeRegion(const FilledElementRegion& region) {
			if (!IsValidFilledShapeRegion(region)) return;

			bool should_rasterize =
				region.type != FilledElementRegion::ShapeType::SolidColor ||
				region.IsComplex();

			bool should_triangulate =
				!should_rasterize &&
				region.contour.points.size() > 4;

			bool is_contour =
				!should_triangulate &&
				region.contour.points.size() <= 4;

			if (should_rasterize)
			{
				AddRasterizedRegion(region);
			}
			else if (should_triangulate)
			{
				AddTriangulatedRegion(region.contour, region.holes, region.solidColor);
			}
			else if (is_contour)
			{
				std::vector<FilledItemContour> contour = { FilledItemContour(region.contour.points) };
				m_group.AddItem<FilledItem>(contour, region.solidColor);
			}
		}

		void SCShapeWriter::AddSlicedElements(const std::vector<FilledElement>& elements, const DOM::Utils::RECT& guides)
		{
			DOM::Utils::RECT elements_bound{
				std::numeric_limits<float>::min(),
				std::numeric_limits<float>::min(),
				std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()
			};

			for (const FilledElement element : elements)
			{
				const DOM::Utils::RECT bound = element.Bound();

				elements_bound.topLeft.x = std::max(bound.topLeft.x, elements_bound.topLeft.x);
				elements_bound.topLeft.y = std::max(bound.topLeft.y, elements_bound.topLeft.y);
				elements_bound.bottomRight.x = std::min(bound.bottomRight.x, elements_bound.bottomRight.x);
				elements_bound.bottomRight.y = std::min(bound.bottomRight.y, elements_bound.bottomRight.y);
			}

			Point<int32_t> image_position_offset(elements_bound.bottomRight.x, elements_bound.bottomRight.y);
			cv::Size image_size(
				ceil(elements_bound.topLeft.x - image_position_offset.x),
				ceil(elements_bound.topLeft.y - image_position_offset.y)
			);

			cv::Mat canvas(image_size, CV_8UC4, cv::Scalar(0x00000000));

			for (const FilledElement& element : elements)
			{
				for (const FilledElementRegion region : element.fill)
				{
					if (!IsValidFilledShapeRegion(region)) continue;

					AddRasterizedRegion(
						region, canvas, image_position_offset
					);
				}

				for (const FilledElementRegion region : element.stroke)
				{
					if (!IsValidFilledShapeRegion(region)) continue;

					AddRasterizedRegion(
						region, canvas, image_position_offset
					);
				}
			}

			const DOM::Utils::MATRIX2D transform = {
				1.0f,
				0.0f,
				0.0f,
				1.0f,
				(FCM::Float)image_position_offset.x,
				(FCM::Float)image_position_offset.y
			};

			m_group.AddItem<SlicedItem>(CreateRef<cv::Mat>(canvas), transform, guides);
		}

		void SCShapeWriter::Finalize(uint16_t id) {
			sc::Shape& shape = m_writer.swf.shapes.emplace_back();
			shape.id = id;

			m_writer.AddGraphicGroup(m_group);
		}
	}
}