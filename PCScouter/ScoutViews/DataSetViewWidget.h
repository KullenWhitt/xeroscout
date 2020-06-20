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
#include "ScoutingDataSet.h"
#include "ViewBase.h"
#include <QSplitter>
#include <QTableWidget>

namespace xero
{
	namespace scouting
	{
		namespace views
		{
			class DataSetViewWidgetItem : public QTableWidgetItem
			{
			public:
				DataSetViewWidgetItem(const QString& str) : QTableWidgetItem(str)
				{
				}

				bool operator<(const QTableWidgetItem& other) const override;
			};

			class SCOUTVIEWS_EXPORT DataSetViewWidget : public QSplitter, public ViewBase
			{
				friend class DataSetViewWidgetItem;

				Q_OBJECT

			public:
				
				DataSetViewWidget(const QString& name, bool editable, QWidget* parent);
				DataSetViewWidget(const QString& name, bool editable, std::function<void(xero::scouting::datamodel::ScoutingDataSet& ds)> fn, QWidget* parent);
				virtual ~DataSetViewWidget();

				void setDataGenerator(std::function<void(xero::scouting::datamodel::ScoutingDataSet& ds)> fn) {
					fn_ = fn;
				}

				xero::scouting::datamodel::ScoutingDataSet& dataset() override {
					return data_;
				}

				virtual void setDataModel(std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> model) {
					ViewBase::setDataModel(model);

					if (model != nullptr)
						model->datasetColumnOrder(name_, colstate_, colgeom_);
				}

				void refreshView();

				void clearView() {
					data_.clear();
					refreshView();
				}

				bool hasDataSet() override {
					return true;
				}

			signals:
				void rowChanged(int row, int col);

			private:
				void updateData(QTableWidget* w);
				void sortData(int column);
				void contextMenuRequested(const QPoint& pt);
				void hideColumn();
				void unhideColumns();
				void resetColumns();

				void columnMoved(int logindex, int oldindex, int newindex);
				void updateColumnOrder();

				void itemChanged(QTableWidgetItem* item);
				void setDelegates();

				void computeHeaderTooltips(QTableWidget* table);

			private:
				QString name_;
				QPoint pt_;

				// Function to generate data set
				std::function<void (xero::scouting::datamodel::ScoutingDataSet &ds)> fn_;
				xero::scouting::datamodel::ScoutingDataSet data_;
				QTableWidget* table_;

				int column_;
				bool direction_;

				QByteArray colstate_;
				QByteArray colgeom_;

				bool editable_;
			};
		}
	}
}
