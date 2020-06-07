//
// Copyright 2020 by Jack W. (Butch) Griffin
//
// Courtesy of Error Code Xero
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
// 



#pragma once

#include "scoutviews_global.h"
#include <QLayout>
#include <qwidget.h>
#include <QList>
#include <QStyle>

namespace xero
{
    namespace scouting
    {
        namespace views
        {

            class SCOUTVIEWS_EXPORT FlowLayout : public QLayout
            {
            public:
                explicit FlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
                explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
                ~FlowLayout();

                void addItem(QLayoutItem* item) override;
                int horizontalSpacing() const;
                int verticalSpacing() const;
                Qt::Orientations expandingDirections() const override;
                bool hasHeightForWidth() const override;
                int heightForWidth(int) const override;
                int count() const override;
                QLayoutItem* itemAt(int index) const override;
                QSize minimumSize() const override;
                void setGeometry(const QRect& rect) override;
                QSize sizeHint() const override;
                QLayoutItem* takeAt(int index) override;

            private:
                int doLayout(const QRect& rect, bool testOnly) const;
                int smartSpacing(QStyle::PixelMetric pm) const;

                QList<QLayoutItem*> itemList;
                int m_hSpace;
                int m_vSpace;
            };
        }
    }
}
