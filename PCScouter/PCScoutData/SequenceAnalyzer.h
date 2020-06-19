#pragma once

#include "pcscoutdata_global.h"
#include "ZebraSequence.h"
#include "RobotActivity.h"
#include <memory>
#include <list>
#include <map>

namespace xero
{
	namespace scouting
	{
		namespace datamodel
		{
			class PCSCOUTDATA_EXPORT SequenceAnalyzer
			{
			public:
				class MatchedSequence
				{
				public:
					MatchedSequence(const QString& name, std::shared_ptr<ZebraSequence> seq, int start, int end)
					{
						name_ = name;
						seq_ = seq;
						start_ = start;
						end_ = end;
					}

					const QString& name() const {
						return name_;
					}

					const std::shared_ptr<ZebraSequence> sequence() const {
						return seq_;
					}

					int start() const {
						return start_;
					}

					double startTime() const {
						return (*seq_)[start_].when();
					}

					int end() const {
						return end_;
					}

					double endTime() const {
						ZebraEvent ev = (*seq_)[end_];
						if (ev.isIdle())
							return ev.when() + ev.duration();

						return ev.when();
					}

				private:
					QString name_;
					int start_;
					int end_;
					std::shared_ptr<ZebraSequence> seq_;
				};

			public:
				SequenceAnalyzer();
				virtual ~SequenceAnalyzer();

				void setSequences(const std::list<std::shared_ptr<ZebraSequence>>& sequences) {
					sequences_.clear();
					for (auto s : sequences)
						sequences_.push_back(s);

					clearAll();
				}

				void clearAll();

				void analyze();

				const std::list<SequenceAnalyzer::MatchedSequence>& matches() const {
					return matches_;
				}

				void setActivities(std::vector<std::shared_ptr<const RobotActivity>> activities) {
					activities_ = activities;
				}

			private:
				void addBlueSequences();
				void analyzeOneSequence(std::shared_ptr<ZebraSequence> seq);

			private:
				std::vector<std::shared_ptr<ZebraSequence>> sequences_;
				std::vector<std::shared_ptr<const RobotActivity>> activities_;
				std::list<MatchedSequence> matches_;
			};
		}
	}
}
