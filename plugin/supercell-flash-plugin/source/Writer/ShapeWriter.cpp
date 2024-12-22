#include "Writer.h"
#include "ShapeWriter.h"
#include "Module/Module.h"

#include <opencv2/opencv.hpp>
#include <CDT.h>

using namespace Animate::Publisher;

namespace sc {
	namespace Adobe {
		void SCShapeWriter::AddGraphic(const Animate::Publisher::SpriteElement& item, const Animate::DOM::Utils::MATRIX2D& matrix) {
			wk::Ref<cv::Mat> image = m_writer.GetBitmap(item);

			m_group.AddItem<SpriteItem>(image, matrix);
		}

		void SCShapeWriter::AddFilledElement(const Animate::Publisher::FilledElement& shape) {
			for (const auto& region : shape.fill) {
				AddFilledShapeRegion(region, shape.transformation);
			}

			for (const auto& region : shape.stroke) {
				AddFilledShapeRegion(region, shape.transformation);
			}
		}

		void SCShapeWriter::AddTriangulatedRegion(
			const Animate::Publisher::FilledElementPath& contour,
			const std::vector<Animate::Publisher::FilledElementPath>& holes,
			const Animate::DOM::Utils::COLOR& color)
		{
			CDT::Triangulation<float> cdt;

			std::vector<CDT::V2d<float>> vertices;
			std::vector<CDT::Edge> edges;

			{
				std::vector<Animate::Publisher::Point2D> points;
				contour.Rasterize(points);

				for (Animate::Publisher::Point2D& point : points)
				{
					vertices.push_back({ point.x, point.y });
				}
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
			for (const auto& hole : holes) {
				std::vector<Animate::Publisher::Point2D> points;
				hole.Rasterize(points);

				for (uint32_t i = 0; points.size() > i; i++) {
					uint32_t secondIndex = vertices.size() + i + 1;
					if (secondIndex >= points.size() + vertices.size()) {
						secondIndex = vertices.size();
					}
					edges.push_back(CDT::Edge(vertices.size() + i, secondIndex));
				}

				for (const auto& point : points) {
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

				std::vector<Animate::Publisher::Point2D> triangle_shape(
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
			const Animate::Publisher::FilledElementRegion& region,
			cv::Mat& canvas,
			Animate::DOM::Utils::RECT bound,
			wk::Point position
		)
		{
			// Corner of current region
			wk::Point local_offset(bound.bottomRight.x, bound.bottomRight.y);

			// Distance between shape and region corners
			wk::Point global_offset(abs(local_offset.x - position.x), abs(local_offset.y - position.y));

			// Size of image to fill
			cv::Size filling_size(abs(bound.bottomRight.x - bound.topLeft.x), abs(bound.bottomRight.y - bound.topLeft.y));

			// Creating fill mask
			cv::Mat canvas_mask(filling_size, CV_8UC1, cv::Scalar(0x00));

			const int contour_shift = 8;

			auto process_points = [
				canvas, &local_offset, &contour_shift
			](const Animate::Publisher::FilledElementPath& path, std::vector<cv::Point>& points)
			{
				std::vector<Animate::Publisher::Point2D> rasterized;
				path.Rasterize(rasterized);

				Animate::Publisher::Point2D center_min(std::numeric_limits<float>().min(), std::numeric_limits<float>().min());
				Animate::Publisher::Point2D center_max(std::numeric_limits<float>().max(), std::numeric_limits<float>().max());

				for (const auto& curve_point : rasterized)
				{
					center_min.x = std::min(curve_point.x, center_min.x);
					center_min.y = std::min(curve_point.y, center_min.y);

					center_max.x = std::max(curve_point.x, center_min.x);
					center_max.y = std::max(curve_point.y, center_min.y);
				}

				Animate::Publisher::Point2D centroid((center_max.x + center_min.x) / 2, (center_max.y + center_min.y) / 2);

				for (const auto& point : rasterized)
				{
					float dx = point.x - centroid.x;
					float dy = point.y - centroid.y;

					float length = std::sqrt(dx * dx + dy * dy);

					// Normalize the vector (to have a length of 1)
					float unitDx = dx / length;
					float unitDy = dy / length;

					const float distance = -1.0f;

					Animate::Publisher::Point2D shrink_point(point.x + unitDx * distance, point.y + unitDy * distance);

					cv::Point& result = points.emplace_back();
					result.x = (shrink_point.x - local_offset.x) * std::pow(2, contour_shift);
					result.y = (shrink_point.y - local_offset.y) * std::pow(2, contour_shift);
				}
			};

				// Contour
				{
					std::vector<cv::Point> points;
					process_points(region.contour, points);

					cv::fillPoly(canvas_mask, points, cv::Scalar(0xFF), cv::LINE_AA, contour_shift);
					//cv::drawContours(canvas_mask, points, -1, cv::Scalar(0xFF), cv::FILLED, cv::LINE_AA);
				}

				{
					for (const auto& path : region.holes)
					{
						std::vector<cv::Point> path_points;
						process_points(path, path_points);

						//cv::drawContours(canvas_mask, path_points, -1, cv::Scalar(0xFF), cv::FILLED, cv::LINE_AA);
						cv::fillPoly(canvas_mask, path_points, cv::Scalar(0x00), cv::LINE_AA, contour_shift);
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
				case Animate::Publisher::FilledElementRegion::ShapeType::SolidColor:
				{
					const cv::Scalar color(
						region.solid.color.blue,
						region.solid.color.green,
						region.solid.color.red,
						region.solid.color.alpha
					);

					filling_image.setTo(color);
				}
				break;
				default:
					break;
				}

				for (int h = 0; filling_size.height > h; h++)
				{
					for (int w = 0; filling_size.width > w; w++)
					{
						cv::Vec4b& origin = canvas.at<cv::Vec4b>(global_offset.y + h, global_offset.x + w);
						cv::Vec4b& destination = filling_image.at<cv::Vec4b>(h, w);
						uchar& mask_alpha = canvas_mask.at<uchar>(h, w);

						if (destination[3] == 0) continue;

						destination[3] = (uchar)std::clamp(destination[3], 0ui8, mask_alpha);

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
			Animate::DOM::Utils::RECT region_bound = region.Bound();
			SCShapeWriter::RoundDomRectangle(region_bound);

			FilledElementRegion local_region = region;
			local_region.Transform(
				{
					SCShapeWriter::RasterizationResolution,
					0.0f,
					0.0f,
					SCShapeWriter::RasterizationResolution,
					0.0f,
					0.0f
				}
			);

			Animate::DOM::Utils::RECT local_bound = local_region.Bound();
			wk::PointF region_offset_x;
			wk::PointF region_offset_y;

			auto round_number = [](float& number, float& offset)
			{
				float base_number = std::trunc(number);
				offset = std::abs(number - base_number);

				if (offset >= 0.5f)
				{
					if (base_number >= 0.0f)
					{
						number = base_number + 1.0f;
					}
					else
					{
						number = base_number - 1.0f;
					}

					return;
				}

				number = base_number;
			};

			round_number(local_bound.topLeft.x, region_offset_y.x);
			round_number(local_bound.topLeft.y, region_offset_y.y);
			round_number(local_bound.bottomRight.x, region_offset_x.x);
			round_number(local_bound.bottomRight.y, region_offset_x.y);

			local_region.Transform(
				{
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					-(std::min(region_offset_x.x, region_offset_y.x)),
					-(std::min(region_offset_x.y, region_offset_y.y))
				}
			);

			wk::Point image_position_offset(local_bound.bottomRight.x, local_bound.bottomRight.y);
			cv::Size image_size(
				ceil(local_bound.topLeft.x - local_bound.bottomRight.x),
				ceil(local_bound.topLeft.y - local_bound.bottomRight.y)
			);

			cv::Mat canvas(image_size, CV_8UC4, cv::Scalar(0x00000000));
			AddRasterizedRegion(local_region, canvas, local_bound, image_position_offset);

			const Animate::DOM::Utils::MATRIX2D transform = {
				1.0f / SCShapeWriter::RasterizationResolution,
				0.0f,
				0.0f,
				1.0f / SCShapeWriter::RasterizationResolution,
				(FCM::Float)region_bound.bottomRight.x,
				(FCM::Float)region_bound.bottomRight.y
			};

			// Adding to group
			m_group.AddItem<SpriteItem>(wk::CreateRef<cv::Mat>(canvas), transform);
		}

		bool SCShapeWriter::IsComplexShapeRegion(const FilledElementRegion& region)
		{
			for (size_t i = 0; region.contour.Count() > i; i++)
			{
				const FilledElementPathSegment& segment = region.contour.GetSegment(i);

				if (segment.SegmentType() != FilledElementPathSegment::Type::Line)
				{
					return true;
				}
			}

			for (const FilledElementPath& path : region.holes)
			{
				for (size_t i = 0; path.Count() > i; i++)
				{
					const FilledElementPathSegment& segment = path.GetSegment(i);

					if (segment.SegmentType() != FilledElementPathSegment::Type::Line)
					{
						return true;
					}
				}
			}

			return false;
		}

		bool SCShapeWriter::IsValidFilledShapeRegion(const FilledElementRegion& region)
		{
			if (region.type == FilledElementRegion::ShapeType::SolidColor)
			{
				// Skip all regions with zero mask_alpha
				if (region.solid.color.alpha <= 0)
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}

		void SCShapeWriter::AddFilledShapeRegion(
			const FilledElementRegion& region,
			const Animate::DOM::Utils::MATRIX2D& matrix
		) {
			if (!IsValidFilledShapeRegion(region)) return;

			bool should_rasterize =
				region.type != FilledElementRegion::ShapeType::SolidColor ||
				IsComplexShapeRegion(region);

			bool is_contour =
				!should_rasterize &&
				region.contour.Count() <= 4 && !(region.contour.Count() > 6) &&
				region.holes.empty();

			bool should_triangulate =
				!should_rasterize &&
				region.contour.Count() > 4;

			FilledElementRegion transformed_region = region;
			transformed_region.Transform(matrix);

			if (should_rasterize)
			{
				AddRasterizedRegion(region);
			}
			else if (is_contour)
			{
				std::vector<Point2D> points;
				region.contour.Rasterize(points);

				std::vector<FilledItemContour> contour = { FilledItemContour(points) };
				m_group.AddItem<FilledItem>(contour, region.solid.color);
			}
			else if (should_triangulate)
			{
				AddTriangulatedRegion(region.contour, region.holes, region.solid.color);
			}
		}

		void SCShapeWriter::AddSlicedElements(const std::vector<FilledElement>& _elements, const Animate::DOM::Utils::RECT& _guides)
		{
			// 9Slice sprites are usually very pixelated
			// So we need to scale their resolution by 2
			// But xy coordinates must be remains the same

			// So first we create a bigger guide
			Animate::DOM::Utils::RECT guides =
			{
				{_guides.topLeft.x * SCShapeWriter::RasterizationResolution, _guides.topLeft.y * SCShapeWriter::RasterizationResolution},
				{_guides.bottomRight.x * SCShapeWriter::RasterizationResolution, _guides.bottomRight.y * SCShapeWriter::RasterizationResolution}
			};

			// Then create copy of element
			// And make their points bigger
			std::vector<FilledElement> elements;
			for (const FilledElement& _element : _elements)
			{
				FilledElement& element = elements.emplace_back(_element);

				element.Transform(
					element.transformation
				);

				element.Transform(
					{
						SCShapeWriter::RasterizationResolution,
						0.0f,
						0.0f,
						SCShapeWriter::RasterizationResolution,
						0.0f,
						0.0f
					}
				);
			}

			Animate::DOM::Utils::RECT elements_bound{
				{std::numeric_limits<float>::min(),
				std::numeric_limits<float>::min()},
				{std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()}
			};

			for (const FilledElement& element : elements)
			{
				const Animate::DOM::Utils::RECT bound = element.Bound();

				elements_bound.topLeft.x = std::max(bound.topLeft.x, elements_bound.topLeft.x);
				elements_bound.topLeft.y = std::max(bound.topLeft.y, elements_bound.topLeft.y);
				elements_bound.bottomRight.x = std::min(bound.bottomRight.x, elements_bound.bottomRight.x);
				elements_bound.bottomRight.y = std::min(bound.bottomRight.y, elements_bound.bottomRight.y);
			}

			SCShapeWriter::RoundDomRectangle(elements_bound);
			wk::Point image_position_offset(
				elements_bound.bottomRight.x,
				elements_bound.bottomRight.y
			);

			cv::Size image_size(
				elements_bound.topLeft.x - image_position_offset.x,
				elements_bound.topLeft.y - image_position_offset.y
			);

			cv::Mat canvas(image_size, CV_8UC4, cv::Scalar(0x00000000));

			for (const FilledElement& element : elements)
			{
				for (const FilledElementRegion region : element.fill)
				{
					if (!IsValidFilledShapeRegion(region)) continue;

					Animate::DOM::Utils::RECT local_bound = region.Bound();
					SCShapeWriter::RoundDomRectangle(local_bound);

					AddRasterizedRegion(
						region, canvas, local_bound, image_position_offset
					);
				}

				for (const FilledElementRegion region : element.stroke)
				{
					if (!IsValidFilledShapeRegion(region)) continue;

					Animate::DOM::Utils::RECT local_bound = region.Bound();
					SCShapeWriter::RoundDomRectangle(local_bound);

					AddRasterizedRegion(
						region, canvas, local_bound, image_position_offset
					);
				}
			}

			const Animate::DOM::Utils::MATRIX2D transform = {
				1 / SCShapeWriter::RasterizationResolution,
				0.0f,
				0.0f,
				1 / SCShapeWriter::RasterizationResolution,
				0,
				0
			};

			m_group.AddItem<SlicedItem>(wk::CreateRef<cv::Mat>(canvas), transform, guides, image_position_offset);
		}

		void SCShapeWriter::RoundDomRectangle(Animate::DOM::Utils::RECT& rect)
		{
			auto round_number = [](float number)
			{
				float base_number = std::trunc(number);
				float decimal = std::abs(number - base_number);

				if (decimal >= 0.5f)
				{
					if (base_number >= 0.0f)
					{
						return base_number + 1.0f;
					}
					else
					{
						return base_number - 1.0f;
					}
				}

				return (float)base_number;
			};

			rect.bottomRight.x = round_number(rect.bottomRight.x);
			rect.bottomRight.y = round_number(rect.bottomRight.y);
			rect.topLeft.x = round_number(rect.topLeft.x);
			rect.topLeft.y = round_number(rect.topLeft.y);
		}

		bool SCShapeWriter::Finalize(uint16_t id, bool required) {
			if (m_group.Size() == 0)
			{
				if (required)
				{
					// Small workaround to avoid crashes with empty shapes but keep it required
					flash::MovieClip& movieclip = m_writer.swf.movieclips.emplace_back();
					movieclip.id = id;

					return true;
				}
				else
				{
					return false;
				}
			}

			flash::Shape& shape = m_writer.swf.shapes.emplace_back();
			shape.id = id;

			m_writer.AddGraphicGroup(m_group);

			return true;
		}
	}
}