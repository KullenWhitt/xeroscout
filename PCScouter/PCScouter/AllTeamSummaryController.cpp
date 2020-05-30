#include "AllTeamSummaryController.h"

using namespace xero::ba;
using namespace xero::scouting::datamodel;

AllTeamSummaryController::AllTeamSummaryController(std::shared_ptr<BlueAlliance> ba, 
	std::shared_ptr<ScoutingDataModel> dm, ScoutingDataSet &ds) : ApplicationController(ba), ds_(ds)
{
	dm_ = dm;

	for (auto t : dm_->teams())
		keys_.push_back(t->key());
	index_ = 0;
	headers_ = false;

	ds_.clear();
}

AllTeamSummaryController::~AllTeamSummaryController()
{
}

bool AllTeamSummaryController::isDone()
{
	return index_ == keys_.count();
}

void AllTeamSummaryController::run()
{
	if (index_ < keys_.count())
	{
		computeOneTeam(keys_.at(index_++));
		pcnt_ = static_cast<int>((double)index_ / (double)keys_.count() * 100);
	}

	if (index_ == keys_.count())
	{
		emit complete(false);
	}
}

void AllTeamSummaryController::computeOneTeam(const QString& key)
{
	QString query, error;
	ScoutingDataSet teamds;

	auto t = dm_->findTeamByKey(key);
	if (t == nullptr)
		return;

	query = "select ";
	bool first = true;
	for (auto f : dm_->getMatchFields())
	{
		if (f->type() != FieldDesc::Type::String)
		{
			if (!first)
				query += ",";
			query += f->name();

			first = false;
		}
	}	
	query += " from matches where MatchTeamKey = '" + key + "'";

	if (!dm_->createCustomDataSet(teamds, query, error))
		return;

	if (!headers_)
	{
		for (auto f : dm_->getTeamFields())
			ds_.addHeader(f);

		for (int col = 0; col < teamds.columnCount(); col++)
			ds_.addHeader(teamds.colHeader(col));

		headers_ = true;
	}

	ds_.newRow();
	for (auto f : dm_->getTeamFields())
	{
		QVariant v = t->value(f->name());
		ds_.addData(v);
	}

	for (int col = 0; col < teamds.columnCount(); col++)
	{
		QVariant v = teamds.columnSummary(col);
		ds_.addData(v);
	}
}