#pragma once

#include "Point.h"

namespace sc
{
	namespace Adobe
	{
		class FilledElementPathSegment
		{
		public:
			class Iterator
			{
			public:
				const FilledElementPathSegment& m_path;
				float m_step = 0;
				float m_it_step = 0;

			public:
				Iterator(const FilledElementPathSegment& path, bool is_end_node = false) : m_path(path) {
					if (!is_end_node)
					{
						m_it_step = m_path.IterationStep();
					}
					else
					{
						m_step = 1.0f;
					}
				}
				Iterator& operator++() {
					m_step += m_it_step;
					return *this;
				}
				Iterator operator++(int) {
					Iterator retval = *this;
					++(*this);
					return retval;
				}
				bool operator==(Iterator& other) const {
					return m_step >= other.m_step;
				}
				bool operator!=(Iterator& other) const {
					return !operator==(other);
				}

				// Data Access
				Point2D operator*() const {
					return m_path.Rasterize(m_step);
				}
				Point2D operator->() const {
					return operator*();
				}

				// Traits
				using iterator_category = std::input_iterator_tag;
				using difference_type = float;
				using value_type = Point2D;
				using pointer = const Point2D*;
				using reference = const Point2D&;
			};

		public:
			enum class Type
			{
				Line,
				Quad,
				Cubic
			};

		public:
			virtual Type SegmentType() const = 0;
			virtual Point2D Rasterize(float t_step) const = 0;
			virtual float IterationStep() const = 0;
			virtual void Transform(const DOM::Utils::MATRIX2D& matrux) = 0;

		public:
			Iterator begin() const { return Iterator(*this); }
			Iterator end() const { return Iterator(*this, true); }

		public:
			bool operator==(const FilledElementPathSegment& other) const;
			bool operator!=(const FilledElementPathSegment& other) const
			{
				return !operator==(other);
			};
		};
	}
}