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

#include "FetchMatchController.h"
#include "BACountResult.h"
#include "BlueAllianceEngine.h"
#include "MatchAlliance.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <memory>

namespace xero
{
	namespace ba
	{
		FetchMatchController::FetchMatchController(BlueAllianceEngine& engine, const QStringList& keys) : KeyListEngineController(engine, keys)
		{
		}

		FetchMatchController::~FetchMatchController()
		{
		}

		QString FetchMatchController::query()
		{
			QString str;
			str = "/match/" + getKey();
			return str;
		}

		BlueAllianceResult::Status FetchMatchController::processJson(int code, std::shared_ptr<QJsonDocument> doc)
		{
			if (doc->isObject())
			{
				QJsonObject obj = doc->object();
				if (obj.contains("key") && obj.value("key").isString() && obj.contains("score_breakdown") && obj.value("score_breakdown").isObject())
				{
					auto matches = engine().matches();
					auto it = matches.find(obj.value("key").toString());
					if (it != matches.end()) {
						it->second->setScoreBreakdown(obj.value("score_breakdown").toObject());
					}
				}
			}

			nextKey();
			return BlueAllianceResult::Status::Success;
		}
	}
}
