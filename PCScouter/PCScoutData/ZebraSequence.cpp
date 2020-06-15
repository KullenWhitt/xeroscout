#include "ZebraSequence.h"
#include <QDebug>
#include <map>
#include <cmath>

namespace xero
{
	namespace scouting
	{
		namespace datamodel
		{
			ZebraSequence::ZebraSequence(const QString& mkey, const QString& tkey, std::shared_ptr<RobotTrack> track, std::vector<std::shared_ptr<const FieldRegion>>& regions)
			{
				mkey_ = mkey;
				tkey_ = tkey;

				extractEvents(track, regions);
				debounceEvents();
				logicallyOrder();
			}

			ZebraSequence::~ZebraSequence()
			{
			}

			void ZebraSequence::swapEvents(int i, int j)
			{
				ZebraEvent tmp = events_[i];
				events_[i] = events_[j];
				events_[j] = tmp;
			}

			void ZebraSequence::logicallyOrder()
			{
				int i = 0;

				while (i < events_.size() - 1)
				{
					if (std::fabs(events_[i].when() - events_[i + 1].when()) < 0.001)
					{
						// This event and the next are at the same time
						if (events_[i].isEnter() && events_[i + 1].isLeave())
							swapEvents(i, i + 1);

						if (events_[i].isIdle() && events_[i + 1].isLeave())
							swapEvents(i, i + 1);
					}
					i++;
				}
			}

			void ZebraSequence::debounceEvents()
			{
				bool changed = true;

				while (changed)
				{
					changed = false;
					for (int i = 0; i < events_.size() - 2; i++)
					{
						const ZebraEvent& ev = events_[i];
						ZebraEvent nextev = ev;

						int j = findNextEvent(i + 1, nextev);
						if (j != -1)
						{
							ZebraEvent nextNextEv = ev;
							int k = findNextEvent(j + 1, nextNextEv);
							if (k != -1)
							{
								//
								// Wee if we have a pattern
								//
								if (nextNextEv.when() - ev.when() < debounceDelay)
								{
									qDebug() << "Collapsed:";
									qDebug() << "    " << ev.toString();
									qDebug() << "    " << nextev.toString();
									qDebug() << "    " << nextNextEv.toString();

									auto it = events_.begin();
									std::advance(it, j);
									events_.erase(it);

									it = events_.begin();
									std::advance(it, i);
									events_.erase(it);

									changed = true;
									break;
								}
							}
						}
					}
				}
			}

			int ZebraSequence::findNextEvent(int i, ZebraEvent& ev)
			{
				while (i < events_.size())
				{
					if (events_[i].name() == ev.name())
					{
						ev = events_[i];
						return i;
					}

					i++;
				}

				return -1;
			}

			void ZebraSequence::extractEvents(std::shared_ptr<RobotTrack> track, std::vector<std::shared_ptr<const FieldRegion>>& regions)
			{
				std::map<QString, bool> states;

				if (!track->hasData())
					return;

				//
				// Establish the time 0 state
				//
				for (auto region : regions)
				{
					bool b = region->isWithin(track->point(0));
					if (b)
					{
						ZebraEvent ev(region->name(), track->time(0), ZebraEvent::EventType::Enter);
						events_.push_back(ev);
					}
					states.insert_or_assign(region->name(), b);
				}

				//
				// Look for state changes and idle periods
				//
				QPointF lastpt;
				double lasttime = 0.0;
				int index = -1;
				for (int i = 1; i < track->timeCount(); i++)
				{
					if (i >= track->pointCount())
						break;

					QPointF here = track->point(i);
					double dist = FieldRegion::distSquared(here, lastpt);

					if (dist > idleThreshold * idleThreshold)
					{
						lastpt = here;
						index = -1;
						lasttime = track->time(i);
					}
					else
					{
						double dt = track->time(i) - lasttime;

						if (dt > idleDelay)
						{
							//
							// We have been in place long enough for an idle event
							//
							if (index != -1)
							{
								//
								// But we already inserted an idle event against this base location so
								// just extend the duration
								//
								ZebraEvent ev = events_[index];
								ev.setDuration(dt);
								events_[index] = ev;
							}
							else
							{
								//
								// This is a new event, so just insert it now
								//
								ZebraEvent ev(track->time(i) - lasttime, lasttime);
								events_.push_back(ev);
								index = events_.size() - 1;
							}
						}
					}

					for (auto region : regions)
					{
						bool b = region->isWithin(track->point(i));
						if (b != states[region->name()])
						{
							//
							// We had a state change
							//
							ZebraEvent ev(region->name(), track->time(i), (b ? ZebraEvent::EventType::Enter : ZebraEvent::EventType::Leave));
							events_.push_back(ev);

							states.insert_or_assign(region->name(), b);
						}
					}
				}
			}
		}
	}
}
