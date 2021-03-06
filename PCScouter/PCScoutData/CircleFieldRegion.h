#pragma once

#include "FieldRegion.h"

namespace xero
{
	namespace scouting
	{
		namespace datamodel
		{
			class CircleFieldRegion : public FieldRegion
			{
			public:
				CircleFieldRegion(const QString &name, QColor c, const QPointF& loc, double radius, xero::scouting::datamodel::Alliance a) : FieldRegion(name, c, a)
				{
					loc_ = loc;
					radius_ = radius;
					bounds_ = QRectF(loc.x() - radius, loc.y() - radius, radius * 2, radius * 2);
				}

				bool isWithin(const QPointF& pt) const override
				{
					return distSquared(pt, loc_) < radius_ * radius_;
				}

				virtual QRectF drawBounds() const {
					return bounds_;
				}

				const QPointF& center() const {
					return loc_;
				}

				double radius() const {
					return radius_;
				}

			private:
				QPointF loc_;
				double radius_;
				QRectF bounds_;
			};
		}
	}
}

