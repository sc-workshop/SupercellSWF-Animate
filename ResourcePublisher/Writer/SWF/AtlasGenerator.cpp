#include "Publisher/AtlasGenerator.h"

#define LIBNEST2D_GEOMETRIES_clipper
#define LIBNEST2D_OPTIMIZER_nlopt
#include <libnest2d/libnest2d.hpp>

namespace sc {
	void AtlasGenerator::NormalizeConfig(AtlasGeneratorConfig& config)
	{
		config.maxSize.first = (uint16_t)clamp((int)config.maxSize.first, 0, MaxTextureDimension);
		config.maxSize.second = (uint16_t)clamp((int)config.maxSize.second, 0, MaxTextureDimension);
		config.scaleFactor = (uint8_t)clamp((int)config.scaleFactor, MinScaleFactor, MaxScaleFactor);
		config.extrude = (uint8_t)clamp((int)config.extrude, MinExtrude, MaxExtrude);
	};

	cv::Mat AtlasGenerator::ImagePreprocess(cv::Mat& src) {
		using namespace cv;

		cv::Mat image = src.clone();
		Size size = src.size();
		int channels = src.channels();

		// Alpha "normalize"
		if (channels == 2 || channels == 4) {
			for (uint16_t h = 0; size.height > h; h++) {
				for (uint16_t w = 0; size.width > w; w++) {
					switch (channels)
					{
					case 4:
					{
						Vec4b pixel = image.at<Vec4b>(h, w);

						if (pixel[3] < 4) {
							image.at<cv::Vec4b>(h, w) = { 0, 0, 0, 0 };
							continue;
						};

						// Alpha premultiply

						float alpha = static_cast<float>(pixel[3]) / 255.0f;
						image.at<Vec4b>(h, w) = {
							static_cast<uchar>(pixel[0] * alpha),
							static_cast<uchar>(pixel[1] * alpha),
							static_cast<uchar>(pixel[2] * alpha),
							pixel[3]
						};
					}
					break;
					case 2:
					{
						Vec2b pixel = image.at<Vec2b>(h, w);

						if (pixel[1] < 4) {
							image.at<cv::Vec2b>(h, w) = { 0, 0 };
							continue;
						};
						float alpha = static_cast<float>(pixel[3]) / 255.0f;
						image.at<Vec2b>(h, w) = {
							static_cast<uchar>(pixel[0] * alpha),
							pixel[1]
						};
					}
					break;
					}
				}
			}
		}

		return image;
	}

	cv::Mat AtlasGenerator::MaskPreprocess(cv::Mat& src) {
		using namespace cv;

		Mat blurred;
		const double sigma = 5, amount = 2.5;

		GaussianBlur(src, blurred, Size(), sigma, sigma);
		Mat sharpened = src * (1 + amount) + blurred * (-amount);
#ifdef CV_DEBUG
		ShowImage("Mask", sharpened);
#endif // CV_DEBUG

		return sharpened;
	}

	vector<cv::Point> AtlasGenerator::GetImageContour(cv::Mat& image)
	{
		using namespace cv;

		vector<Point> result;

		vector<vector<Point>> contours;
		findContours(image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for (vector<Point>& points : contours) {
#ifdef CV_DEBUG
			Mat drawingImage;
			cvtColor(image, drawingImage, COLOR_GRAY2BGR);
			ShowContour(drawingImage, points);
#endif

			move(points.begin(), points.end(), back_inserter(result));
		}

		SnapToBorder(image, result);

		return result;
	}

	void AtlasGenerator::ExtrudePoints(cv::Mat src, vector<cv::Point>& points) {
		using namespace cv;

		const uint16_t offsetX = (uint16_t)PercentOf(src.cols, 5);
		const uint16_t offsetY = (uint16_t)PercentOf(src.rows, 5);

		const int centerW = (int)ceil(src.cols / 2);
		const int centerH = (int)ceil(src.rows / 2);

		for (Point& point : points) {
			bool isEdgePoint = (point.x == 0 || point.x == src.cols) && (point.y == 0 || point.y == src.rows);

			if (!isEdgePoint) {
				int x = point.x - centerW;
				int y = point.y - centerH;

				if (x >= 0) {
					x += offsetX;
				}
				else {
					x -= offsetX;
				}

				if (y >= 0) {
					y += offsetY;
				}
				else {
					y -= offsetY;
				}

				point = {
					clamp(x + centerW, 0, src.cols),
					clamp(y + centerH, 0, src.rows),
				};
			}
		}
	}

	void AtlasGenerator::SnapToBorder(cv::Mat src, vector<cv::Point>& points) {
		using namespace cv;

		const double snapPercent = 7;

		// Snaping variables
		const uint16_t minW = (uint16_t)ceil(PercentOf(src.cols, snapPercent));
		const uint16_t maxW = src.cols - minW;

		const uint16_t minH = (uint16_t)ceil(PercentOf(src.rows, snapPercent));
		const uint16_t maxH = src.rows - minH;

		for (Point& point : points) {
			if (minW > point.x) {
				point.x = 0;
			}
			else if (point.x >= maxW) {
				point.x = src.cols;
			}

			if (minH > point.y) {
				point.y = 0;
			}
			else if (point.y >= maxH) {
				point.y = src.rows;
			}
		}
	}

	cv::Mat AtlasGenerator::GetImagePolygon(AtlasGeneratorItem& item, AtlasGeneratorConfig& config)
	{
		using namespace cv;

		cv::Mat image = AtlasGenerator::ImagePreprocess(item.image);

		Mat polygonMask;
		switch (image.channels())
		{
		case 4:
			extractChannel(image, polygonMask, 3);
			break;
		case 2:
			extractChannel(image, polygonMask, 1);
			break;
		default:
			polygonMask = Mat(image.size(), CV_8UC1, Scalar(255));
			break;
		}

		Rect imageBounds = boundingRect(polygonMask);
		if (imageBounds.width <= 0) imageBounds.width = 1;
		if (imageBounds.height <= 0) imageBounds.height = 1;

		// Image croping by alpha
		image = image(imageBounds);
		polygonMask = polygonMask(imageBounds);

		Size srcSize = item.image.size();
		Size dstSize = polygonMask.size();

		if (IsRectangle(image, config)) {
			item.polygon = vector<AtlasGeneratorVertex>(4);

			item.polygon[0].uv = { 0, 0 };
			item.polygon[1].uv = { 0, dstSize.height };
			item.polygon[2].uv = { dstSize.width, dstSize.height };
			item.polygon[3].uv = { dstSize.width, 0 };

			item.polygon[0].xy = { imageBounds.x, imageBounds.y };
			item.polygon[1].xy = { imageBounds.x, imageBounds.y + dstSize.height };
			item.polygon[2].xy = { imageBounds.x + dstSize.width, imageBounds.y + dstSize.height };
			item.polygon[3].xy = { imageBounds.x + dstSize.width, imageBounds.y };

			return image;
		}
		else {
			item.polygon.clear();
		}

		polygonMask = AtlasGenerator::MaskPreprocess(polygonMask);
		vector<Point> contour = GetImageContour(polygonMask);

		vector<cv::Point> polygon;
		ExtrudePoints(polygonMask, polygon);
		convexHull(contour, polygon, true);

		if (config.scaleFactor > 1) {
			Size imageSize(
				(int)ceil(image.cols / config.scaleFactor),
				(int)ceil(image.rows / config.scaleFactor));

			imageSize.width = clamp(imageSize.width, 1, MaxTextureDimension);
			imageSize.height = clamp(imageSize.height, 1, MaxTextureDimension);

			resize(image, image, imageSize);
		}

		for (const Point point : polygon) {
			item.polygon.push_back(
				AtlasGeneratorVertex
				(
					point.x + imageBounds.x, point.y + imageBounds.y,
					(uint16_t)ceil(point.x / config.scaleFactor), (uint16_t)ceil(point.y / config.scaleFactor)
				)
			);
		}

#ifdef CV_DEBUG
		ShowContour(image, item.polygon);
#endif

		return image;
	};

	bool AtlasGenerator::IsRectangle(cv::Mat& image, AtlasGeneratorConfig& config)
	{
		using namespace cv;

		Size size = image.size();
		int channels = image.channels();

		// Image has no alpha. It makes no sense to generate a polygon
		if (channels == 1 || channels == 3) {
			return true;
		}

		if (size.width < (config.maxSize.first * 3 / 100) && size.height < (config.maxSize.second * 3 / 100)) {
			return true;
		}

		return false;
	};

	void AtlasGenerator::PlaceImageTo(cv::Mat& src, cv::Mat& dst, uint16_t x, uint16_t y)
	{
		using namespace cv;

		Size srcSize = src.size();
		Size dstSize = dst.size();

		for (uint16_t h = 0; srcSize.height > h; h++) {
			uint16_t dstH = h + y;
			if (dstH >= dstSize.height) continue;

			for (uint16_t w = 0; srcSize.width > w; w++) {
				uint16_t dstW = w + x;
				if (dstW >= dstSize.width) continue;

				Vec4b pixel(0, 0, 0, 0);

				switch (src.channels())
				{
				case 4:
					pixel = src.at<cv::Vec4b>(h, w);
					break;
				default:
					break;
				}

				if (pixel[3] == 0) {
					continue;
				}

				switch (dst.channels())
				{
				case 4:
					dst.at<Vec4b>(dstH, dstW) = pixel;
					break;
				default:
					break;
				}
			}
		}
	};

	uint32_t AtlasGenerator::SearchDuplicate(vector<AtlasGeneratorItem>& items, cv::Mat& image, uint32_t range) {
		using namespace cv;

		for (uint32_t i = 0; range > i; i++) {
			Mat& other = items[i].image;

			if (CompareImage(image, other)) {
				return i;
			}
		}

		return UINT32_MAX;
	}

	bool AtlasGenerator::CompareImage(cv::Mat src1, cv::Mat src2) {
		using namespace cv;

		if (src1.cols != src2.cols || src1.rows != src2.rows) return false;
		int imageChannelsCount = src1.channels();
		int otherChannelsCount = src2.channels();

		if (imageChannelsCount != otherChannelsCount) return false;

		vector<Mat> channels(imageChannelsCount);
		vector<Mat> otherChannels(imageChannelsCount);
		split(src1, channels);
		split(src2, otherChannels);

		size_t pixelCount = src1.rows * src1.cols;
		for (int j = 0; imageChannelsCount > j; j++) {
			for (int w = 0; src1.cols > w; w++) {
				for (int h = 0; src1.rows > h; h++) {
					uchar pix = channels[j].at<uchar>(h, w);
					uchar otherPix = otherChannels[j].at<uchar>(h, w);
					if (pix != otherPix) {
						return false;
					}
				}
			}
		}

		return true;
	}

	AtlasGeneratorResult AtlasGenerator::Generate(vector<AtlasGeneratorItem>& items, vector<cv::Mat>& atlases, AtlasGeneratorConfig& config) {
		using namespace libnest2d;
		NormalizeConfig(config);

		// Duplicated images
		vector<size_t> duplicates;

		// Vector with polygons for libnest2d
		vector<Item> packerItems;

		// Atlas sprites
		vector<cv::Mat> images;

		for (uint32_t i = 0; items.size() > i; i++) {
			AtlasGeneratorItem& item = items[i];
			uint32_t imageIndex = SearchDuplicate(items, item.image, i);

			if (imageIndex == UINT32_MAX) {
				// TODO remove later
				if (item.image.channels() != 4) return AtlasGeneratorResult::BAD_IMAGE;

				if (item.image.rows <= 0 || item.image.cols <= 0) {
					return AtlasGeneratorResult::BAD_IMAGE;
				}

				// Polygon generation
				cv::Mat atlasSprite = GetImagePolygon(item, config);
				if (atlasSprite.cols >= config.maxSize.first || atlasSprite.rows >= config.maxSize.second) {
					return AtlasGeneratorResult::TOO_BIG_IMAGE;
				}

				images.push_back(atlasSprite);

				if (item.polygon.size() <= 0) {
					return AtlasGeneratorResult::BAD_POLYGON;
				}

				// Adding new items to packer
				libnest2d::Item packerItem = libnest2d::Item(vector<ClipperLib::IntPoint>(item.polygon.size() + 1), {});

				for (uint16_t p = 0; packerItem.vertexCount() > p; p++) {
					if (p == item.polygon.size()) { // End point for libnest
						packerItem.setVertex(p, { item.polygon[0].uv.first, item.polygon[0].uv.second });
					}
					else {
						packerItem.setVertex(p, { item.polygon[p].uv.first, item.polygon[p].uv.second });
					}
				}

				packerItems.push_back(packerItem);
				duplicates.push_back(SIZE_MAX);
			}
			else {
				duplicates.push_back(imageIndex);
			}
		}

		NestConfig<BottomLeftPlacer, DJDHeuristic> cfg;
		//cfg.placer_config.epsilon = config.extrude;
		cfg.placer_config.allow_rotations = true;

		libnest2d::NestControl control;
		control.progressfn = config.progress;
		size_t binCount = nest(packerItems, Box(config.maxSize.first, config.maxSize.second, { (int)ceil(config.maxSize.first / 2), (int)ceil(config.maxSize.second / 2) }), config.extrude, cfg, control);
		if (binCount >= 0xFF) return AtlasGeneratorResult::TOO_MANY_IMAGES;

		// Texture preparing
		vector<cv::Size> textureSize(binCount);
		for (size_t i = 0; packerItems.size() > i; i++) {
			Item& item = packerItems[i];
			if (item.binId() == libnest2d::BIN_ID_UNSET) return AtlasGeneratorResult::BAD_POLYGON;

			auto shape = item.transformedShape();
			auto box = item.boundingBox();

			cv::Size& size = textureSize[item.binId()];

			auto x = getX(box.maxCorner());
			auto y = getY(box.maxCorner());

			if (x > size.height) {
				size.height = (int)x;
			}
			if (y > size.width) {
				size.width = (int)y;
			}
		}

		for (cv::Size& size : textureSize)
		{
			atlases.push_back(
				cv::Mat(
					size.width,
					size.height,
					(int)config.textureType,
					cv::Scalar(0)
				)
			);
		}

		uint32_t itemOffset = 0;
		for (uint32_t i = 0; items.size() > i; i++) {
			bool isDuplicate = duplicates[i] != SIZE_MAX;
			if (isDuplicate) {
				items[i] = items[duplicates[i]];
				itemOffset++;
				continue;
			}

			Item& packerItem = packerItems[i - itemOffset];
			AtlasGeneratorItem& item = items[i];
			cv::Mat& atlas = atlases[packerItem.binId()];

			auto rotation = packerItem.rotation();
			double rotationAngle = -(rotation.toDegrees());

			auto shape = packerItem.transformedShape();
			auto box = packerItem.boundingBox();

			// Point processing
			item.textureIndex = static_cast<uint8_t>(packerItem.binId());
			for (size_t j = 0; item.polygon.size() > j; j++) {
				uint16_t x = item.polygon[j].uv.first;
				uint16_t y = item.polygon[j].uv.second;

				uint16_t u = (uint16_t)ceil(x * rotation.cos() - y * rotation.sin() + getX(packerItem.translation()));
				uint16_t v = (uint16_t)ceil(y * rotation.cos() + x * rotation.sin() + getY(packerItem.translation()));

				item.polygon[j].uv = { u, v };
			}

			// Image processing
			cv::Mat sprite = images[i - itemOffset].clone();
			if (rotationAngle != 0) {
				cv::Point2f center((float)((sprite.cols - 1) / 2.0), (float)((sprite.rows - 1) / 2.0));
				cv::Mat rot = cv::getRotationMatrix2D(center, rotationAngle, 1.0);
				cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), sprite.size(), (float)rotationAngle).boundingRect2f();

				rot.at<double>(0, 2) += bbox.width / 2.0 - sprite.cols / 2.0;
				rot.at<double>(1, 2) += bbox.height / 2.0 - sprite.rows / 2.0;

				cv::warpAffine(sprite, sprite, rot, bbox.size(), cv::INTER_NEAREST);
			}

			cv::copyMakeBorder(sprite, sprite, config.extrude, config.extrude, config.extrude, config.extrude, cv::BORDER_REPLICATE);

			auto x = getX(box.minCorner());
			auto y = getY(box.minCorner());
			PlaceImageTo(
				sprite,
				atlas,
				static_cast<uint16_t>(x - config.extrude),
				static_cast<uint16_t>(y - config.extrude)
			);
		}

#ifdef CV_DEBUG
		vector<cv::Mat> sheets;
		cv::RNG rng = cv::RNG(time(NULL));

		for (cv::Mat& atlas : atlases) {
			sheets.push_back(cv::Mat(atlas.size(), CV_8UC4, cv::Scalar(0)));
		}

		for (AtlasGeneratorItem& item : items) {
			vector<cv::Point> polyContour;
			for (AtlasGeneratorVertex point : item.polygon) {
				polyContour.push_back(cv::Point(point.uv.first, point.uv.second));
			}
			cv::Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			fillPoly(sheets[item.textureIndex], polyContour, color);
		}

		for (cv::Mat& sheet : sheets) {
			ShowImage("Sheet", sheet);
		}

		for (cv::Mat& atlas : atlases) {
			ShowImage("Atlas", atlas);
		}

		cv::destroyAllWindows();
#endif // DEBUG

		return AtlasGeneratorResult::OK;
	}
}