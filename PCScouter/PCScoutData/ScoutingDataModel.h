//
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

/// \file ScoutingDataModel.h
///       This file contains the top level data model for the scouting database


#pragma once

#include "pcscoutdata_global.h"
#include "ScoutingForm.h"
#include "ScoutingDataSet.h"
#include "ScoutingDatabase.h"
#include "DataModelTeam.h"
#include "DataModelMatch.h"
#include "TabletIdentity.h"
#include "SyncHistoryRecord.h"
#include "GraphDescriptorCollection.h"
#include "GraphDescriptor.h"
#include <QString>
#include <QJsonDocument>
#include <QFile>
#include <QObject>
#include <QJsonObject>
#include <list>
#include <memory>

namespace xero
{
	namespace scouting
	{
		namespace datamodel
		{
			class TeamDataSummary;

			/// The data model for the scouting data
			class PCSCOUTDATA_EXPORT ScoutingDataModel : public QObject
			{
				friend class JSonDataModelConverter;

				Q_OBJECT

			public:
				///\brief the argument to the modelChanged signal.  Indicates the type of change
				enum class ChangeType {
					HistoryChanged,					///< the history of changes changed
					TabletListChanged,				///< the list of pit or match scouting tablets was changed
					TeamAdded,						///< a new team was added to the data model
					TeamDataChanged,				///< the data associated with a team was changed
					PitScoutingDataAdded,			///< pit scouting data was added or replaced for a team
					PitScoutingTabletChanged,		///< the tablet used for pit scouting a team was changed
					MatchScoutingDataAdded,			///< match scouting data was added or replaced for a match
					MatchScoutingTabletChanged,		///< the tablet used for match scouting a match was changed
					TeamAddedToMatch,				///< added a team to a match
					MatchAdded,						///< a new match was added to the data model
					MatchDataChanged,				///< the data associated with a match was changed
					Reset,							///< the data model was reset
					ZebraDataAdded,					///< zebra data was added to the matches
					GraphDescriptor,				///< the graph descriptors were changed
				};

			public:
				/// \brief create a new data model
				/// \param evkey the event Blue Alliance key
				/// \param evname the event name
				/// \param pitform the name of the file containing the pit scouting form
				/// \param matchform the name of the file contining the match scouting form
				ScoutingDataModel(const QString& evkey, const QString& evname, std::shared_ptr<ScoutingForm> pit, std::shared_ptr<ScoutingForm> match);

				/// \brief create a new data model
				ScoutingDataModel();

				/// \brief destroy this data model
				virtual ~ScoutingDataModel();

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// These method query the data model but do not change them.  They will never cause a modelChanged event.
				//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


				/// \brief return the Blue Alliance event key
				/// \returns the blue alliance event key
				const QString& evkey() const {
					return ev_key_;
				}

				/// \brief return the graph view descriptors
				/// \returns the graph descriptors
				const GraphDescriptorCollection& graphDescriptors() const {
					return graph_descriptor_;
				}

				/// \brief returns the event name
				/// \returns the event name
				const QString& evname() const {
					return event_name_;
				}

				/// \brief returns the history of synchronizations made to the data model
				/// \returns a list of synchronizations made to the data model
				std::list<SyncHistoryRecord> history() const {
					std::list<SyncHistoryRecord> ret;

					for (auto h : history_)
						ret.push_back(h);

					return ret;
				}

				/// \brief returns the list of teams in the datamodel
				/// \returns the list of teams in the datamodel
				std::list<std::shared_ptr<const DataModelTeam>> teams() const {
					std::list<std::shared_ptr<const DataModelTeam>> list;

					for (auto t : teams_)
						list.push_back(t);

					return list;
				}

				/// \brief returns a shared DataModelTeam pointer given the key for the team
				/// \param key the Blue Alliance key for the desired team
				/// \returns a shared DataModelTeam pointer given the key for the team
				std::shared_ptr<const DataModelTeam> findTeamByKey(const QString& key) const {
					for (auto t : teams_) {
						if (t->key() == key)
							return t;
					}

					return nullptr;
				}

				/// \brief find a team given its team number
				/// \param num the number of the team to find
				/// \returns a with with the given team number
				std::shared_ptr<const DataModelTeam> findTeamByNumber(int num) const {
					for (auto t : teams_) {
						if (t->number() == num)
							return t;
					}

					return nullptr;
				}

				/// \brief find a match given the Blue Alliance key for the match
				/// \param key the blue alliance key for the match
				/// returns a shared pointer to the match if a match with the given key was found, otherwise nullptr
				std::shared_ptr<const DataModelMatch> findMatchByKey(const QString& key) const {
					for (auto m : matches_) {
						if (m->key() == key)
							return m;
					}

					return nullptr;
				}

				/// \brief return a list of the matches in the data model
				/// \returns a list of the matches in the data model
				std::list<std::shared_ptr<const DataModelMatch>> matches() const {
					std::list<std::shared_ptr<const DataModelMatch>> ret;

					for (auto m : matches_)
						ret.push_back(m);

					return ret;
				}

				/// \brief find a match given its type, set and match
				/// \param type the type of match (qm, qf, sf, f)
				/// \param set the set number
				/// \param match the match number
				/// \returns a pointer to the match if a match with the given type, set, and match was found, otherwise returns null
				std::shared_ptr<DataModelMatch> findMatchByTriple(const QString& type, int set, int match) const {
					for (auto m : matches_) {
						if (m->compType() == type && m->set() == set && m->match() == match)
							return m;
					}

					return nullptr;
				}

				/// \brief returns the set of tablets for match scouting
				/// \returns the set of tablets for match scouting
				const QStringList& matchTablets() {
					return match_tablets_;
				}

				/// \brief returns the set of tablets for pit scouting
				/// \returns the set of tablets for pit scouting
				const QStringList& pitTablets() {
					return pit_tablets_;
				}

				/// \brief returns true if the data model has changed since it was last saved
				/// \returns true if the data model has changed since it was last saved
				bool isDirty() const {
					return dirty_;
				}

				/// \brief returns a pointer to the match scouting form
				/// \returns a pointer to the match scouting form
				std::shared_ptr<const ScoutingForm> matchScoutingForm() const {
					return match_scouting_form_;
				}

				/// \brief returns a pointer to the pit scouting form
				/// \returns a pointer to the pit scouting form
				std::shared_ptr<const ScoutingForm> pitScoutingForm() const {
					return pit_scouting_form_;
				}

				/// \brief returns all field names across the pit scouting form, the match scouting form, and the blue alliance data
				/// \returns all field names ;
				QStringList getAllFieldNames() const;

				QStringList getPitFieldNames() const;

				/// \brief returns fields that contain per match data 
				/// \returns fields that are valid per match data
				QStringList getMatchFieldNames() const;

				/// \brief returns true if the blue alliance data is loaded
				/// \returns true if blue alliance data is loaded
				bool isBlueAllianceDataLoaded() const;

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// These methods generate data sets.  The are a special case of the query methods in that they synthesis sets of data
				// from the datamodel.  These methods may have significant runtimes.
				//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				/// \brief create a dataset for the matches in the datamodel
				/// Iterate over all of the matches and create a dataset.  The rows are the matches.  The columns are
				/// the scouting data for each match.  The scouting data is the data defined in the match scouting form
				/// as well as the data loaded for each match from the Blue Alliance if the match data has been imported.
				/// \param set the dataset to contain the data.  Any existing data will be cleared.
				void createMatchDataSet(ScoutingDataSet& set) const;

				/// \brief create a dataset for the pits in the datamodel.
				/// Iterate over all of the teams anad create a dataset for pit scouting data.  The rows are the teams and
				/// the columns are the scouting data fields from the pit scouting form
				/// \param set the dataset to contain the data.  Any existing data will be cleared.
				void createPitDataSet(ScoutingDataSet& set) const;

				/// \brief create a dataset for a custom SQL query
				/// The rows and columns are defined by the results of the query
				/// \param set the dataset to contain the data.  Any existing data will be cleared.
				/// \param query the SQL query to execute
				/// \param error the error message if the SQL query was invalid
				bool createCustomDataSet(ScoutingDataSet& set, const QString& query, QString& error) const;

				/// \brief create summary data for a single team.
				/// \param key the Blue Alliance key for the team of interest
				/// \param result the team summary data result.
				/// \returns true if the summary was generated, false if an error occurs
				bool createTeamSummaryData(const QString& key, TeamDataSummary& result);

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// These method change the datamodel and will generate modelChanged signals
				//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void setTeamOPR(const QString& teamkey, double opr) {
					auto team = findTeamByKeyInt(teamkey);
					team->setOPR(opr);
				}

				/// \brief set the various graph views
				/// \param grarray an array of group view descriptors as JSON
				/// \param err any returned error message
				/// \returns true if the info is updated
				bool setGraphViews(const QJsonArray& grarray, QString &err) {
					return graph_descriptor_.load(grarray, err);
				}

				void setTeamRanking(const QString& key, const QJsonObject &obj) {
					auto team = findTeamByKeyInt(key);
					team->setRanking(obj);
					
					dirty_ = true;
					emitChangedSignal(ChangeType::TeamDataChanged);
				}

				/// \brief add a team to a match.
				/// This is used when constructing a match and therefore teams are expected to be added in the
				/// position order for the match.
				/// \param matchkey the Blue Alliance key for the match to add the team to
				/// \param c the color of the alliance (red or blue)
				/// \param teamkey the Blue Alliance key for the team
				/// \returns false if the match does not exist, or if it has teams already otherwise true
				bool addTeamToMatch(const QString& matchkey, DataModelMatch::Alliance c, const QString& teamkey) {
					auto dm = findMatchByKeyInt(matchkey);
					if (dm == nullptr)
						return false;

					//if (dm->teams(c).size() >= 3)
						//return false;

					dm->addTeam(c, teamkey);

					dirty_ = true;
					emitChangedSignal(ChangeType::TeamAddedToMatch);

					return true;
				}

				/// \brief assign a tablet to a team for pit scouting
				/// \param key the Blue Alliance key for the team
				/// \param the tablet name for the tablet to assign
				/// \returns true if the team key indicates a valid team, false otherwise
				bool assignTeamTablet(const QString& key, const QString& tablet)
				{
					auto team = findTeamByKeyInt(key);
					if (team == nullptr)
						return false;

					team->setTablet(tablet);

					dirty_ = true;

					emitChangedSignal(ChangeType::PitScoutingTabletChanged);

					return true;
				}

				/// \brief set the scouting data for a team
				/// \param key the Blue Alliance key for the team
				/// \param data the scouting data for the team
				/// \retursn true if the team key indicates a valid team, false otherwise
				bool setTeamScoutingData(const QString& key, ScoutingDataMapPtr data, bool replace = false)
				{
					auto team = findTeamByKeyInt(key);
					if (team == nullptr)
						return false;

					bool ret = team->setPitScoutingData(data, replace);
					if (ret)
					{
						dirty_ = true;
						emitChangedSignal(ChangeType::PitScoutingDataAdded);
					}

					return ret;
				}

				/// \brief assign a tablet to a match
				/// \param key the Blue Alliance key for a match
				/// \param color the color of the alliance for this tablet
				/// \param slot the position (1, 2, 3) for the tablet
				/// \param tablet the tablet name to assign
				/// \returns false if the match key is invalid, or the slot is not between 1 - 3
				bool assignMatchTablet(const QString& key, DataModelMatch::Alliance color, int slot, const QString& tablet)
				{
					auto match = findMatchByKeyInt(key);
					if (match == nullptr)
						return false;

					if (slot < 1 || slot > 3)
						return false;

					match->setTablet(color, slot, tablet);

					dirty_ = true;

					emitChangedSignal(ChangeType::MatchScoutingTabletChanged);

					return true;
				}

				/// \brief assign scouting data to a match
				/// \param key the Blue Alliance key for a match
				/// \param color the color of the alliance for this tablet
				/// \param slot the position (1, 2, 3) for the tablet
				/// \param data the scouting data to assign
				/// \param false if true, replace all scouting data with this one entry
				/// \returns false if the match key is invalid, or the slot is not between 1 - 3
				bool setMatchScoutingData(const QString& key, DataModelMatch::Alliance color, int slot, ScoutingDataMapPtr data, bool replace = false)
				{
					bool ret;

					auto match = findMatchByKeyInt(key);
					if (match == nullptr)
						return false;

					if (slot < 1 || slot > 3)
						return false;

					ret = match->setScoutingData(color, slot, data, replace);
					if (ret)
					{
						dirty_ = true;
						emitChangedSignal(ChangeType::MatchScoutingDataAdded);
					}

					return ret;
				}

				/// \brief set the lists of tablets to use for scouting
				/// This method should only be called on a data model that does not have
				/// pit tablets or match tablets assigned.  This method is used for
				/// constructing a data model.
				/// \returns true if sucessful and false if the pit or match tablet scouting list where not empty
				bool setTabletLists(const QStringList& pits, const QStringList& matches) {
					if (pit_tablets_.size() > 0 || match_tablets_.size() > 0)
						return false;

					pit_tablets_ = pits;
					match_tablets_ = matches;

					dirty_ = true;
					emitChangedSignal(ChangeType::TabletListChanged);

					return true;
				}

				/// \brief add a new team to the data model
				/// \param key the Blue Alliance key for this team
				/// \param number the team number for the team
				/// \param the nickname for teh team
				/// \returns a shared pointer to the team data model object that was added
				std::shared_ptr<DataModelTeam> addTeam(const QString& key, int number, const QString& name) {
					auto t = findTeamByKeyInt(key);
					if (t != nullptr) {
						if (t->name() != name || t->number() != number) {
							t->setName(name);
							t->setNumber(number);

							dirty_ = true;
							emitChangedSignal(ChangeType::TeamDataChanged);
						}
						return t;
					}

					t = std::make_shared<DataModelTeam>(key, number, name);
					teams_.push_back(t);

					dirty_ = true;
					emitChangedSignal(ChangeType::TeamAdded);

					return t;
				}

				/// \brief add a new match to the data model.
				/// If a match with the given key already exists, update the information for the match if necessary and
				/// return a shared pointer to the match.  Otherwise, create the match and added it.  Return a shared pointer
				/// to the new match.
				/// \param key the Blue Alliance key for the match
				/// \param comp the type of match (qm, qf, sf, f)
				/// \param set the set number
				/// \param match the match number
				/// \param the expected time for the match
				/// \returns a shared pointer to the match
				std::shared_ptr<DataModelMatch> addMatch(const QString& key, const QString& comp, int set, int match, int etime) {
					auto m = findMatchByKeyInt(key);
					if (m != nullptr)
					{
						if (m->compType() != comp || m->set() != set || m->match() != match)
						{
							m->setCompType(comp);
							m->setSet(set);
							m->setMatch(match);
							dirty_ = true;

							emitChangedSignal(ChangeType::MatchDataChanged);
						}
						return m;
					}

					m = std::make_shared<DataModelMatch>(key, comp, set, match, etime);
					matches_.push_back(m);

					dirty_ = true;
					emitChangedSignal(ChangeType::MatchAdded);

					return m;
				}

				/// \brief add zebra data to data model
				/// \param key the match key for the match
				/// \param obj the JSON object containing zebra data (in blue alliance format)
				void assignZebraData(const QString& key, const QJsonObject& obj) {
					auto m = findMatchByKeyInt(key);
					m->setZebra(obj);

					emitChangedSignal(ChangeType::ZebraDataAdded);
				}

				void updateGraphDescriptor(const GraphDescriptor& desc) {
					graph_descriptor_.update(desc);
					dirty_ = true;

					emitChangedSignal(ChangeType::GraphDescriptor);
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// These method are file I/O methods for the data model
				//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				/// \brief returns the filename associated with the data model
				/// \returns the filename associted with the data models
				const QString& filename() {
					return filename_;
				}

				/// \brief set the filename for saving the datamodel
				/// \param filename the filename for the data model
				void setFilename(const QString& filename) {
					filename_ = filename;
				}

				/// \brief resets the filename to empty
				void resetFilename() {
					filename_.clear();
				}

				/// \brief save the data model to the default file
				/// If there is no default file, return false
				/// \returns true if the data was saved, otherwise returns false
				bool save() {

					if (filename_.length() == 0)
						return false;

					QJsonDocument doc = generateAllJSONDocument();
					QFile file(filename_);

					if (!file.open(QIODevice::WriteOnly))
						return false;

					QByteArray data;

					if (filename_.endsWith(".evd"))
						data = doc.toBinaryData();
					else
						data = doc.toJson(QJsonDocument::JsonFormat::Indented);

					if (file.write(data) != data.size())
					{
						file.close();
						return false;
					}

					file.close();
					dirty_ = false;
					return true;
				}

				/// \brief save the data model to the file name given
				/// \returns true if the data was saved, otherwise false
				bool save(const QString& filename) {
					filename_ = filename;
					return save();
				}

				/// \brief load the datamodel from the file given
				/// Note, the modelChanged signal is disabled during the load operation as this would
				/// trigger signals for each data model loaded from file.  It is restored when the load
				/// operation is complete.
				/// \returns true if the data model was loaded successfully
				bool load(const QString& filename);

				/// \brief load the scouting data from the file given
				/// Note, the modelChanged signal is disabled during the load operation as this would
				/// trigger signals for each data model loaded from file.  It is restored when the load
				/// operation is complete.
				/// \returns true if the data model was loaded successfully
				bool loadScoutingDataJSON(const QString& filename);

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// These method are misc methods
				//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				/// \brief remove all but the latest scouting data
				void removeOldScoutingData();

				/// \brief assign tablets to matches for match scouting
				void assignMatches();

				/// \brief assign tablets to pits for pit scouting
				void assignPits();

				/// \brief given a team key, return the team number
				/// \param key the Blue Alliance key for the team
				/// \returns the team number
				int teamKeyToNumber(const QString& key);

				/// \brief generate a JSON document that represents all of the data in the data model
				/// \returns a json documentthat represents all of the data in the data model
				QJsonDocument generateAllJSONDocument();

				/// \brief generate a JSON document that represents all of the data in the data model except scouting data
				/// \returns a json documentthat represents all of the data in the data model except scouting data
				QJsonDocument generateCoreJSONDocument(bool extra = true);

				/// \brief generate a JSON document that represents the scouting data in the data model for a specific tablet
				/// \param sender the identity of the tablet requesting the data, incorporated as part of the data sent from the tablet to the central, can be null
				/// \param the tablet we want data for
				/// \returns a JSON document that represents the scouting data in the data model
				QJsonDocument generateScoutingData(const TabletIdentity* sender, const QString& tablet);

				/// \brief load all of the json data in the document into the data model.
				/// The JSON data is expected to be of type 'all' which means both core data and scouting data.
				/// Note if this method returns false, partial data may have loaded into the data model.  Use save()
				/// and load() to save and restore the model if necessary.
				/// \param doc the json document to load
				/// \param error if this method returns false, the errors that occurred are stored in this list
				/// \returns true if the data was loaded into the data model, false otherwise
				bool loadAllJSON(const QJsonDocument& doc, QStringList& error);

				/// \brief load the json data in the document into the data model
				/// The JSON data is expected to be of type 'core' which means just core data.
				/// Note if this method returns false, partial data may have loaded into the data model.  Use save()
				/// and load() to save and restore the model if necessary.
				/// \param doc the json document to load
				/// \param error if this method returns false, the errors that occurred are stored in this list
				/// \returns true if the data was loaded into the data model, false otherwise
				bool loadCoreJSON(const QJsonDocument& doc, QStringList& error);

				/// \brief load the json data in the document into the data model
				/// The JSON data is expected to be of type 'scouting' which means just scouting data
				/// Note if this method returns false, partial data may have loaded into the data model.  Use save()
				/// and load() to save and restore the model if necessary.
				/// \param doc the json document to load
				/// \param error if this method returns false, the errors that occurred are stored in this list
				/// \returns true if the data was loaded into the data model, false otherwise
				bool loadScoutingDataJSON(const QJsonDocument& doc, TabletIdentity& tablet, QString& error, QStringList& pits, QStringList& matches);

				/// \brief reset the data in the data model
				/// This destroys all data in the data model.  Wse with care
				void reset() {
					filename_.clear();

					ev_key_.clear();
					event_name_.clear();

					match_scouting_form_ = nullptr;
					pit_scouting_form_ = nullptr;
					teams_.clear();
					matches_.clear();

					registered_tablets_.clear();

					pit_tablets_.clear();
					match_tablets_.clear();

					history_.clear();

					dirty_ = false;
					emitChangedSignal(ChangeType::Reset);
				}

				/// \brief generate randome data to fill all scouting data in the model.
				/// Used primarimaly for testing the application.  The mod parameter is used to
				/// tell which items get data.  A value of 1 will generate data for all items.  A value
				/// of two will generate data for 1/2.  3 for 1/3, etc.
				/// \param profile the data profile for generating random data
				/// \mod the modulo used to tell which items get random data
				void generateRandomScoutingData(GameRandomProfile& profile, int mod);

				/// \brief break out per robot blue alliance data and add to each robots scouting data for a match.
				/// The key for the map given is the Blue Alliance match key.  The value for the map is a pair, where the first
				/// entry is the Blue Alliance scoring breakdown data for the red alliance and the second entry is the Blue Alliance
				/// scoring breakdown data for the blue alliance.  Any data in this map that applies to matches in the data model is
				/// pulled out and assigned to the match scouting data for the appropriate robot.
				/// \param data a map of data from the blue alliance.
				void breakoutBlueAlliancePerRobotData(std::map<QString, std::pair<ScoutingDataMapPtr, ScoutingDataMapPtr>>& data);

				/// \brief remove any scouting data associated with the given tablet
				/// \param tablet the tablet we want to remove data for
				void removeScoutingData(const QString& tablet);

				/// \brief return the the registered tablets assocaited with the data model.
				/// This is the list of the identities of every tablet that has ever synchronized with the data model.
				/// \returns the list of tablet identities
				const std::list<TabletIdentity> registeredTablets() const {
					return registered_tablets_;
				}

				/// \brief register the given tablet with the data model
				/// \param id the tablet identity to register
				void registerTablet(const TabletIdentity& id) {
					registered_tablets_.push_back(id);
				}

				void addHistoryRecord(const SyncHistoryRecord& rec) {
					history_.push_back(rec);

					dirty_ = true;
					emitChangedSignal(ChangeType::HistoryChanged);
				}

				/// \brief if signals are enabled, signal any listener that the data model changed
				/// \param type the type of the change made
				void emitChangedSignal(ChangeType type) {
					if (emit_changes_)
					{
						auto db = ScoutingDatabase::getInstance();
						db->clearAllTables();

						emit modelChanged(type);
					}
				}

			signals:
				void modelChanged(ChangeType type);

			protected:
				/// \brief returns a shared DataModelTeam pointer given the key for the team
				/// \param key the Blue Alliance key for the desired team
				/// \returns a shared DataModelTeam pointer given the key for the team
				std::shared_ptr<DataModelTeam> findTeamByKeyInt(const QString& key) {
					for (auto t : teams_) {
						if (t->key() == key)
							return t;
					}

					return nullptr;
				}

				std::shared_ptr<DataModelMatch> findMatchByKeyInt(const QString& key) {
					for (auto m : matches_) {
						if (m->key() == key)
							return m;
					}

					return nullptr;
				}

			private:
				void incrMatchTabletIndex(int& index) {
					index++;
					if (index == match_tablets_.size())
						index = 0;
				}

				void incrPitTabletIndex(int& index) {
					index++;
					if (index == pit_tablets_.size())
						index = 0;
				}

				std::shared_ptr<DataModelMatch> findMatch(const QString& comp, int set, int match);

				std::shared_ptr<ScoutingDataMap> generateRandomData(GameRandomProfile& gen, std::shared_ptr<ScoutingForm> form);
				void breakOutBAData(std::shared_ptr<DataModelMatch> m, DataModelMatch::Alliance c, ScoutingDataMapPtr data);

				void processDataSetAlliance(ScoutingDataSet& set, std::shared_ptr<DataModelMatch> m, DataModelMatch::Alliance c) const;

			private:
				QString filename_;

				bool dirty_;

				QString ev_key_;
				QString event_name_;

				std::shared_ptr<ScoutingForm> match_scouting_form_;
				std::shared_ptr<ScoutingForm> pit_scouting_form_;

				std::list<std::shared_ptr<DataModelTeam>> teams_;
				std::list<std::shared_ptr<DataModelMatch>> matches_;

				std::list<TabletIdentity> registered_tablets_;

				QStringList pit_tablets_;
				QStringList match_tablets_;

				std::list<SyncHistoryRecord> history_;

				bool emit_changes_;

				GraphDescriptorCollection graph_descriptor_;
			};

		}
	}
}
