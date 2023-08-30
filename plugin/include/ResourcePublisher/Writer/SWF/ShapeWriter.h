#pragma once

#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"
#include "ApplicationFCMPublicIDs.h"

#include "string"

namespace sc {
	namespace Adobe {
		class SCWriter;
		struct Sprite;

		class SCShapeWriter : public SharedShapeWriter {
			SCWriter* m_writer = nullptr;
			std::vector<Sprite> m_sprites;

		public:
			SCShapeWriter(SCWriter* writer) : m_writer(writer) {};

			void Init(Context& context, SymbolBehaviorInfo& info);

			void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix);

			void AddSolidColorRegion(FilledShapePath* contour, std::vector<FilledShapePath*> holes, cv::Scalar color);
			void AddRasterizedSolidColorRegion(FilledShapePath* contour, std::vector<FilledShapePath*> holes, cv::Scalar color);

			void AddFilledShape(const FilledShape& shape);
			void AddFilledShapeRegion(const FilledShapeRegion& region);

			void Finalize(uint16_t id);
		};
	}
}