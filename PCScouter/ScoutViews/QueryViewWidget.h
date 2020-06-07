//
// Copyright 2020 by Jack W. (Butch) Griffin
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
#include "DataSetViewWidget.h"
#include "ScoutingDataModel.h"
#include "ScoutingDatabase.h"
#include "ViewBase.h"
#include <QPushButton>
#include <QBoxLayout>
#include <QLineEdit>
#include <QWidget>
#include <QCompleter>

namespace xero
{
	namespace scouting
	{
		namespace views
		{
			class SCOUTVIEWS_EXPORT QueryViewWidget : public QWidget, public ViewBase
			{
			public:
				QueryViewWidget(QWidget* parent);
				virtual ~QueryViewWidget();

				void clearView();
				void refreshView();

				void madeActive();

				bool hasDataSet() override {
					return true;
				}

				xero::scouting::datamodel::ScoutingDataSet& dataset() {
					return data_view_->dataset();
				}

			private:
				void executeQuery();

			private:
				QLineEdit* query_widget_;
				QPushButton* query_execute_;
				DataSetViewWidget* data_view_;
				QCompleter* completer_;
			};

		}
	}
}
