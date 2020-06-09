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

#include "BlueAlliance.h"
#include "BlueAllianceEngine.h"
#include "ScoutingDataModel.h"
#include "ScoutingDataMap.h"
#include "ScoutingForm.h"
#include <memory>

class DataModelBuilder
{
public:
	DataModelBuilder() = delete;
	~DataModelBuilder() = delete;

	static void addBlueAllianceData(std::shared_ptr<xero::ba::BlueAlliance> ba, std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> dm, int maxmatch);

	static std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> 
		buildModel(xero::ba::BlueAllianceEngine& engine, std::shared_ptr<xero::scouting::datamodel::ScoutingForm> team,
		std::shared_ptr<xero::scouting::datamodel::ScoutingForm> match, const QString& evkey, QString& error);

	static std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel>
		buildModel(xero::ba::BlueAllianceEngine& engine, std::shared_ptr<const xero::scouting::datamodel::ScoutingForm> team,
			std::shared_ptr<const xero::scouting::datamodel::ScoutingForm> match, const QString& evkey, QString& error);

	static std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel>
		buildModel(xero::ba::BlueAllianceEngine& engine, const QString &pitform, const QString &matchform, 
		const QString& evkey, QString& error);

	static bool addTeamsMatches(xero::ba::BlueAllianceEngine& engine, std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> dm);

	static bool addTeams(xero::ba::BlueAllianceEngine& engine, std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> dm, const QStringList &keys);

	static void jsonToPropMap(const QJsonObject& obj, const QString& alliance, xero::scouting::datamodel::ScoutingDataMapPtr map);

private:
	static void breakoutBlueAlliancePerRobotData(std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> dm, 
		std::map<QString, std::pair<xero::scouting::datamodel::ScoutingDataMapPtr, xero::scouting::datamodel::ScoutingDataMapPtr>>& data, int maxmatch);

	static void breakOutBAData(std::shared_ptr<xero::scouting::datamodel::ScoutingDataModel> dm,
		std::shared_ptr<const xero::scouting::datamodel::DataModelMatch> m, xero::scouting::datamodel::Alliance c,
		xero::scouting::datamodel::ScoutingDataMapPtr data);
};

