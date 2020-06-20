//
// Copyright(C) 2020 by Jack (Butch) Griffin
//
//	This program is free software : you can redistribute it and /or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see < https://www.gnu.org/licenses/>.
//
//
//
// This work we create by the named individual(s) above in support of the
// FRC robotics team Error Code Xero.
//

#pragma once

#include "scoutviews_global.h"
#include "ScoutingForm.h"
#include "ScoutingDataModel.h"
#include "ViewBase.h"
#include "GameFieldManager.h"
#include "ImageManager.h"
#include "FormView.h"
#include <QStackedWidget>
#include <QTreeWidget>
#include <map>

namespace xero
{
	namespace scouting
	{
		namespace views
		{
			class SCOUTVIEWS_EXPORT DocumentView : public QStackedWidget
			{
				Q_OBJECT

			public:
				enum class ViewType : int
				{
					NoModelView = 0,
					TeamScoutingFormView = 1,
					MatchScoutingFormViewRed = 2,
					MatchScoutingFormViewBlue = 3,
					TeamView = 4,
					MatchView = 5,
					MatchDataSet = 6,
					TeamDataSet = 7,
					CustomDataSet = 8,
					TeamReport = 9,
					AllTeamReport = 10,
					HistoryView = 11,
					MergeListView = 12,
					ZebraDataView = 13,
					MatchGraphView = 14,
					AllianceGraphView = 15,
					PickListView = 16,
					ZebraAnalysis = 17,
					ZebraPatternEditor=18,
					ZebraRegionEditor=19,
					ZebraIntro=20,
					Predictions = 21,
					FirstFormView = 22
				};

			public:
				DocumentView(xero::scouting::datamodel::ImageManager& mgr, int year, const QString &tablet, QWidget* parent = Q_NULLPTR);
				~DocumentView();

				void setTablet(const QString& tablet) {
					for (int i = 0; i < count(); i++)
					{
						QWidget* w = widget(i);
						ViewBase* vb = dynamic_cast<ViewBase*>(w);
						if (vb != nullptr)
							vb->setTablet(tablet);
					}
				}

				void setDataModel(std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> model) {
					for (int i = 0; i < count(); i++)
					{
						QWidget* w = widget(i);
						ViewBase* vb = dynamic_cast<ViewBase*>(w);
						if (vb != nullptr)
							vb->setDataModel(model);
					}

					needsRefreshAll();
				}

				void clearAll();
				void refreshAll();
				void needsRefreshAll();

				ViewType viewType() {
					return view_;
				}

				void setViewType(ViewType index);

				QWidget* getWidget(int index);

				QWidget* getWidget(ViewType t) {
					return getWidget(static_cast<int>(t));
				}

				QWidget* currentWidget() {
					return getWidget(view_);
				}

				void deleteWidget(int index);

				bool isForm(int index) {
					return index > static_cast<int>(ViewType::CustomDataSet);
				}

				bool createFetchFormView(const QString& key, const QString& title, QColor c, 
						FormView::FormType type, xero::scouting::datamodel::Alliance a, int& index);

				void logText(const QString& msg) {
					emit logMessage(msg);
				}

			signals:
				void itemDoubleClicked(ViewType type, const QString& key);
				void logMessage(const QString& msg);

			private:
				void teamItemDoubleClicked(QTreeWidgetItem* item, int col);
				void matchItemDoubleClicked(QTreeWidgetItem* item, int col);

			private:
				ViewType view_;
				std::map<QString, QWidget*> scouting_forms_;
				GameFieldManager field_mgr_;
				xero::scouting::datamodel::ImageManager &images_;
			};
		}
	}
}
