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

#include "ScoutingDataMap.h"
#include "Alliance.h"
#include <QJsonObject>
#include <QString>
#include <QVariant>
#include <vector>
#include <memory>
#include <cassert>

namespace xero
{
	namespace scouting
	{
		namespace datamodel
		{
			class DataModelMatch
			{
				friend class ScoutingDataModel;
				friend class JSonDataModelConverter;

			public:
				DataModelMatch(const QString& key, const QString& comp, int set, int match, int etime) {
					key_ = key;
					comp_type_ = comp;
					set_number_ = set;
					match_number_ = match;
					etime_ = etime;
				}

				virtual ~DataModelMatch() {
				}

				bool hasBlueAllianceData() const {
					if (blue_ba_data_.size() != 3 || red_ba_data_.size() != 3)
						return false;

					for (int i = 0; i < 3; i++)
					{
						if (blue_ba_data_[i]->size() == 0 || red_ba_data_[i]->size() == 0)
							return false;
					}

					return true;
				}

				QString title() const {
					QString ret;

					if (comp_type_ == "qm")
						ret = "Qual Match";
					else if (comp_type_ == "qf")
						ret = "Quarter Finals";
					else if (comp_type_ == "sf")
						ret = "Semi Finals";
					else if (comp_type_ == "ef")
						ret = "Einstein Finals";
					else
						ret = "Finals";

					if (comp_type_ != "qm")
						ret += ", Set " + QString::number(set_number_);

					ret += ", Match " + QString::number(match_number_);

					return ret;
				}

				int eventTime() const {
					return etime_;
				}

				bool needsTablet(const QString& tab) const {
					auto bit = std::find(blue_tablets_.begin(), blue_tablets_.end(), tab);
					auto rit = std::find(red_tablets_.begin(), red_tablets_.end(), tab);

					return bit != blue_tablets_.end() || rit != red_tablets_.end();
				}

				const QString& key() const {
					return key_;
				}

				static QString toString(Alliance c) {
					QString ret;

					if (c == Alliance::Red)
						ret = "red";
					else
						ret = "blue";

					return ret;
				}

				static Alliance allianceFromString(const QString& str) {
					assert(str == "red" || str == "blue");

					if (str == "red")
						return Alliance::Red;

					return Alliance::Blue;
				}

				QStringList allTeams() const {
					QStringList ret;

					for (const QString& s : red_)
						ret << s;

					for (const QString& s : blue_)
						ret << s;

					return ret;
				}

				const QString& compType() const {
					return comp_type_;
				}

				int set() const {
					return set_number_;
				}

				int match() const {
					return match_number_;
				}

				bool hasScoutingData(Alliance a, int slot) const {
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					bool ret = false;

					if (slot >= 1 && slot <= 3) {
						if (a == Alliance::Red) {
							ret = (red_scouting_data_[slot - 1].size() > 0);
						}
						else {
							ret = (blue_scouting_data_[slot - 1].size() > 0);
						}
					}

					return ret;
				}

				QVariant value(Alliance a, int slot, const QString& name) const {
					QVariant ret;
					ConstScoutingDataMapPtr scout, ba;

					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
					{
						if (red_scouting_data_[slot - 1].size() > 0)
							scout = red_scouting_data_[slot - 1].back();

						if (red_ba_data_.size() > 0)
							ba = red_ba_data_[slot - 1];
					}
					else
					{
						if (blue_scouting_data_[slot - 1].size() > 0)
							scout = blue_scouting_data_[slot - 1].back();

						if (blue_ba_data_.size() > 0)
							ba = blue_ba_data_[slot - 1];

					}

					if (scout != nullptr)
					{
						auto it = scout->find(name);
						if (it != scout->end())
							return it->second;
					}

					if (ba != nullptr)
					{
						auto it = ba->find(name);
						if (it != ba->end())
							return it->second;
					}

					return ret;
				}

				ScoutingDataMapPtr data(Alliance a, int slot) const {
					auto data = std::make_shared<ScoutingDataMap>();

					ConstScoutingDataMapPtr scout, ba ;

					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
					{
						if (red_scouting_data_[slot - 1].size() > 0)
							scout = red_scouting_data_[slot - 1].back();

						if (red_ba_data_.size() > 0)
							ba = red_ba_data_[slot - 1];
					}
					else
					{
						if (blue_scouting_data_[slot - 1].size() > 0)
							scout = blue_scouting_data_[slot - 1].back();

						if (blue_ba_data_.size() > 0)
							ba = blue_ba_data_[slot - 1];

					}

					if (scout != nullptr)
					{
						for (auto pair : *scout)
							data->insert_or_assign(pair.first, pair.second);
					}

					if (ba != nullptr)
					{
						for (auto pair : *ba)
							data->insert_or_assign(pair.first, pair.second);
					}

					return data;
				}

				std::list<ConstScoutingDataMapPtr> scoutingDataList(Alliance a, int slot) const {
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					std::list<ConstScoutingDataMapPtr> ret;

					if (a == Alliance::Red)
					{
						for (auto d : red_scouting_data_[slot - 1])
							ret.push_back(d);
					}
					else
					{
						for (auto d : blue_scouting_data_[slot - 1])
							ret.push_back(d);
					}
					return ret;
				}

				bool tabletToAllianceSlot(const QString& tabname, Alliance& color, int& slot) const {
					color = Alliance::Red;
					slot = -1;

					for (int i = 0; i < red_tablets_.size(); i++) {
						if (red_tablets_[i] == tabname)
						{
							color = Alliance::Red;
							slot = i + 1;
							return true;
						}
					}

					for (int i = 0; i < blue_tablets_.size(); i++) {
						if (blue_tablets_[i] == tabname)
						{
							color = Alliance::Blue;
							slot = i + 1;
							return true;
						}
					}

					return false;
				}

				bool teamToAllianceSlot(const QString& teamkey, Alliance& color, int& slot) const {
					color = Alliance::Red;
					slot = -1;

					for (int i = 0; i < red_tablets_.size(); i++) {
						if (red_[i] == teamkey)
						{
							color = Alliance::Red;
							slot = i + 1;
							return true;
						}
					}

					for (int i = 0; i < blue_tablets_.size(); i++) {
						if (blue_[i] == teamkey)
						{
							color = Alliance::Blue;
							slot = i + 1;
							return true;
						}
					}

					return false;
				}

				const QString& team(Alliance a, int slot) const {
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
						return red_[slot - 1];

					return blue_[slot - 1];
				}

				const QString& tablet(Alliance a, int slot) const {
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
						return red_tablets_[slot - 1];

					return blue_tablets_[slot - 1];
				}


				void removeOldScoutingData() {
					for (int i = 0; i < 3; i++) {
						while (red_scouting_data_[i].size() > 1)
							red_scouting_data_[i].erase(red_scouting_data_[i].begin());
					}

					for (int i = 0; i < 3; i++) {
						while (blue_scouting_data_[i].size() > 1)
							blue_scouting_data_[i].erase(blue_scouting_data_[i].begin());
					}
				}

				const QJsonObject& zebra() const {
					return zebra_;
				}

			protected:
				ConstScoutingDataMapPtr scoutingData(Alliance a, int slot) const {
					ConstScoutingDataMapPtr ret = nullptr;

					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
					{
						if (red_scouting_data_[slot - 1].size() > 0)
							ret = red_scouting_data_[slot - 1].back();
					}
					else
					{
						if (blue_scouting_data_[slot - 1].size() > 0)
							ret = blue_scouting_data_[slot - 1].back();
					}

					return ret;
				}

				ConstScoutingDataMapPtr blueAllianceData(Alliance a, int slot) const {
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
						return red_ba_data_[slot - 1];

					return blue_ba_data_[slot - 1];
				}

				void setZebra(const QJsonObject& data) {
					zebra_ = data;
				}

				void setCompType(const QString& ct) {
					comp_type_ = ct;
				}

				void setSet(int n) {
					set_number_ = n;
				}

				void setMatch(int m) {
					match_number_ = m;
				}

				void clearTeams() {
					red_.clear();
					red_tablets_.clear();
					blue_.clear();
					blue_tablets_.clear();
				}

				void setBlueAllianceData(Alliance a, int slot, ScoutingDataMapPtr map) {
					if (a == Alliance::Red)
						red_ba_data_[slot - 1] = map;
					else
						blue_ba_data_[slot - 1] = map;
				}

				void addTeam(Alliance a, const QString& t) {
					if (a == Alliance::Red) {
						red_.push_back(t);
						red_tablets_.push_back("");
						if (red_scouting_data_.size() < red_.size())
							red_scouting_data_.resize(red_.size());
						red_ba_data_.push_back(nullptr);
						assert(red_.size() <= 3);
					}
					else {
						blue_.push_back(t);
						blue_tablets_.push_back("");
						if (blue_scouting_data_.size() < blue_.size())
							blue_scouting_data_.resize(blue_.size());
						blue_ba_data_.push_back(nullptr);
						assert(blue_.size() <= 3);
					}
				}


				void setTablet(Alliance c, int slot, const QString& tablet) {
					assert(slot >= 1 && slot <= 3);
					if (c == Alliance::Red) {
						red_tablets_[slot - 1] = tablet;
					}
					else {
						blue_tablets_[slot - 1] = tablet;
					}
				}

				void clearScoutingData(Alliance a, int slot)
				{
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);

					if (a == Alliance::Red)
						red_scouting_data_[slot - 1].clear();
					else
						blue_scouting_data_[slot - 1].clear();
				}

				bool setScoutingData(Alliance a, int slot, ScoutingDataMapPtr data, bool replace = false) {
					assert(slot >= 1 && slot <= 3);
					assert(a == Alliance::Red || a == Alliance::Blue);
					assert(data != nullptr);

					if (a == Alliance::Red)
					{
						if (replace) 
						{
							red_scouting_data_[slot - 1].clear();
							red_scouting_data_[slot - 1].push_back(data);
						}
						else {
							for (auto sd : red_scouting_data_[slot - 1])
							{
								if (compareTwoMaps(sd, data))
									return false;
							}
							red_scouting_data_[slot - 1].push_back(data);
						}
					}
					else
					{
						if (replace)
						{
							blue_scouting_data_[slot - 1].clear();
							blue_scouting_data_[slot - 1].push_back(data);
						}
						else
						{
							for (auto sd : blue_scouting_data_[slot - 1])
							{
								if (compareTwoMaps(sd, data))
									return false;
							}
							blue_scouting_data_[slot - 1].push_back(data);
						}
					}
					return true;
				}

			private:
				class OneRobot
				{
				public:
				private:
					QString team_key_;
					QString tablet_;
				};

			private:
				//
				// The blue alliance key for this match
				//
				QString key_;

				//
				// The type of competition, qual match (qm), quarter final (qf), semi final (qf), final (f), or einstein final (ef)
				//
				QString comp_type_;

				//
				// The set number, -1 for qual matches
				//
				int set_number_;

				//
				// The match number
				//
				int match_number_;

				//
				// The event time for the match
				//
				int etime_;

				//
				// The team keys for the three teams on the red alliance
				//
				std::vector<QString> red_;

				//
				// The three red tablets
				//
				std::vector<QString> red_tablets_;

				//
				// The list of scouting data for each team in the red alliance.  Note, there is a list
				// for the case where there are multiple sources of data, but the last set of data is used in all
				// queries.  The merge operation can be used to consolidate the data into a single set.
				//
				std::vector<std::list<ScoutingDataMapPtr>> red_scouting_data_;

				//
				// The blue alliance data for this match for the red alliance, broken out by robot
				//
				std::vector<ScoutingDataMapPtr> red_ba_data_;

				//
				// The team keys for the three teams on the blue alliance
				//
				std::vector<QString> blue_;

				//
				// The three blue tablets
				//
				std::vector<QString> blue_tablets_;

				//
				// The list of scouting data for each team in the blue alliance.  Note, there is a list
				// for the case where there are multiple sources of data, but the last set of data is used in all
				// queries.  The merge operation can be used to consolidate the data into a single set.
				//
				std::vector<std::list<ScoutingDataMapPtr>> blue_scouting_data_;

				//
				// The blue alliance data for this match for the red alliance, broken out by robot
				//
				std::vector<ScoutingDataMapPtr> blue_ba_data_;

				//
				// The zebra data for the match (stored as the JSON from the blue alliance)
				//
				QJsonObject zebra_;
			};
		}
	}
}
