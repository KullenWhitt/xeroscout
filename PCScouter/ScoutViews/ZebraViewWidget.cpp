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
//

#include "ZebraViewWidget.h"
#include "RobotTrack.h"
#include "CircleFieldRegion.h"
#include "RectFieldRegion.h"
#include "PolygonFieldRegion.h"
#include <QBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QInputDialog>
#include <functional>

using namespace xero::scouting::datamodel;

namespace xero
{
	namespace scouting
	{
		namespace views
		{
			ZebraViewWidget::ZebraViewWidget(QWidget* parent) : QWidget(parent), ViewBase("ZebraViewWidget")
			{
				QVBoxLayout* vlay = new QVBoxLayout();
				setLayout(vlay);

				QWidget* top = new QWidget(this);
				vlay->addWidget(top);

				QHBoxLayout* hlay = new QHBoxLayout();
				top->setLayout(hlay);

				mode_select_ = new QComboBox(top);
				hlay->addWidget(mode_select_);
				mode_select_->addItem("Tracks", static_cast<int>(PathFieldView::ViewMode::Track));
				mode_select_->addItem("Heatmap", static_cast<int>(PathFieldView::ViewMode::Heatmap));
				mode_select_->addItem("Replay", static_cast<int>(PathFieldView::ViewMode::Robot));
				(void)connect(mode_select_, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &ZebraViewWidget::modeChanged);

				matches_ = new QRadioButton("Match", top);
				matches_->setChecked(true);
				hlay->addWidget(matches_);
				(void)connect(matches_, &QRadioButton::toggled, this, &ZebraViewWidget::matchesSelected);

				robot_ = new QRadioButton("Robot", top);
				hlay->addWidget(robot_);
				(void)connect(robot_, &QRadioButton::toggled, this, &ZebraViewWidget::robotSelected);

				box_ = new QComboBox(top);
				hlay->addWidget(box_);
				(void)connect(box_, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &ZebraViewWidget::comboxChanged);
				QSizePolicy p(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
				box_->setSizePolicy(p);

				vertical_ = new QSplitter(Qt::Orientation::Horizontal, this);
				vlay->addWidget(vertical_);
				p = QSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
				vertical_->setSizePolicy(p);

				field_ = new PathFieldView(vertical_);
				connect(field_, &PathFieldView::showContextMenu, this, &ZebraViewWidget::fieldContextMenu);
				field_->setViewMode(PathFieldView::ViewMode::Track);
				vertical_->addWidget(field_);

				horizontal_ = new QSplitter(Qt::Orientation::Vertical, vertical_);
				vertical_->addWidget(horizontal_);

				QWidget* top_right = new QWidget(horizontal_);
				QVBoxLayout *vlay2 = new QVBoxLayout();
				top_right->setLayout(vlay2);
				horizontal_->addWidget(top_right);

				all_ = new QCheckBox(top_right);
				vlay2->addWidget(all_);
				connect(all_, &QCheckBox::stateChanged, this, &ZebraViewWidget::checkChanged);

				list_ = new QListWidget(top_right);
				vlay2->addWidget(list_);
				list_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
				connect(list_, &QListWidget::itemChanged, this, &ZebraViewWidget::detailItemChanged);

				info_ = new QTreeWidget(horizontal_);
				info_->setHeaderHidden(true);
				info_->setColumnCount(1);
				horizontal_->addWidget(info_);

				slider_ = new TimeBoundWidget(0.0, 135.0, this);
				vlay->addWidget(slider_);
				connect(slider_, &TimeBoundWidget::rangeChanged, this, &ZebraViewWidget::rangeChanged);
				connect(slider_, &TimeBoundWidget::changeAnimationState, this, &ZebraViewWidget::sliderChangedAnimationState);

				mode_ = Mode::Uninitialized;
				animation_timer_ = nullptr;
			}

			ZebraViewWidget::~ZebraViewWidget()
			{
			}

			void ZebraViewWidget::fieldContextMenu(QPoint pt)
			{
				menu_point_ = pt;

				if (field_->viewMode() == PathFieldView::ViewMode::Robot)
				{
					QAction* act;
					QMenu* menu = new QMenu("Field");

					act = menu->addAction("Show Defense");
					act->setCheckable(true);
					if (field_->showDefense())
						act->setChecked(true);
					else
						act->setChecked(false);
					connect(act, &QAction::triggered, this, &ZebraViewWidget::defenseToggled);

					QMenu* all = new QMenu("Add Highlight");
					menu->addMenu(all);

					QMenu* both = new QMenu("Both Alliances");

					auto cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Both, HighlightType::Circle);
					act = both->addAction("Add Circular Area");
					connect(act, &QAction::triggered, cb);
					
					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Both, HighlightType::Rectangle);
					act = both->addAction("Add Rectangular Area");
					connect(act, &QAction::triggered, cb);

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Both, HighlightType::Polygon);
					act = both->addAction("Add Polygon Area");
					connect(act, &QAction::triggered, cb);

					all->addMenu(both);

					QMenu* red = new QMenu("Red Alliance");

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Red, HighlightType::Circle);
					act = red->addAction("Add Circular Area");
					connect(act, &QAction::triggered, cb);

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Red, HighlightType::Rectangle);
					act = red->addAction("Add Rectangular Area");
					connect(act, &QAction::triggered, cb);

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Red, HighlightType::Polygon);
					act = red->addAction("Add Polygon Area");
					connect(act, &QAction::triggered, cb);

					all->addMenu(red);

					QMenu* blue = new QMenu("Blue Alliance");

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Blue, HighlightType::Circle);
					act = blue->addAction("Add Circular Area");
					connect(act, &QAction::triggered, cb);

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Blue, HighlightType::Rectangle);
					act = blue->addAction("Add Rectangular Area");
					connect(act, &QAction::triggered, cb);

					cb = std::bind(&ZebraViewWidget::addHighlight, this, Alliance::Blue, HighlightType::Polygon);
					act = blue->addAction("Add Polygon Area");
					connect(act, &QAction::triggered, cb);

					all->addMenu(blue);

					bool items = false;
					QMenu* remove = new QMenu("Remove");
					for (auto h : dataModel()->fieldRegions())
					{
						act = remove->addAction(h->name());
						auto removedb = std::bind(&ZebraViewWidget::removeHighlight, this, h->name());
						connect(act, &QAction::triggered, removedb);
						items = true;
					}

					if (items)
						menu->addMenu(remove);

					menu->exec(pt);
				}
			}

			void ZebraViewWidget::defenseToggled()
			{
				field_->setShowDefense(!field_->showDefense());
			}

			void ZebraViewWidget::polygonSelected(const std::vector<QPointF>& points, Alliance a)
			{
				bool ok;

				disconnect(field_connect_);

				QString name = QInputDialog::getText(this, "Name", "Name", QLineEdit::Normal, "", &ok);
				if (ok)
				{
					QColor c = QColor(242, 245, 66, 128);

					if (a == Alliance::Red)
						c = QColor(255, 0, 0, 128);
					else if (a == Alliance::Blue)
						c = QColor(0, 0, 255, 128);

					auto h = std::make_shared<PolygonFieldRegion>(name, c, points, a);
					field_->addHighlight(h);
					dataModel()->addFieldRegion(h);
				}
			}

			void ZebraViewWidget::areaSelected(const QRectF& area, Alliance a, HighlightType ht)
			{
				bool ok;

				disconnect(field_connect_);

				QString name = QInputDialog::getText(this, "Name", "Name", QLineEdit::Normal, "", &ok);
				if (ok)
				{
					QColor c = QColor(242, 245, 66, 128);

					if (a == Alliance::Red)
						c = QColor(255, 0, 0, 128);
					else if (a == Alliance::Blue)
						c = QColor(0, 0, 255, 128);

					if (ht == HighlightType::Circle)
					{
						qDebug() << "ZebraViewWidget::areaSelected " << area;
						auto h = std::make_shared<CircleFieldRegion>(name, c, area.center(), area.width() / 2, a);
						field_->addHighlight(h);
						dataModel()->addFieldRegion(h);
					}
					else
					{
						qDebug() << "ZebraViewWidget::areaSelected " << area;
						auto h = std::make_shared<RectFieldRegion>(name, c, area, a);
						field_->addHighlight(h);
						dataModel()->addFieldRegion(h);
					}
				}
			}

			void ZebraViewWidget::removeHighlight(const QString& name)
			{
				std::shared_ptr<const FieldRegion> region = nullptr;

				for (auto h : dataModel()->fieldRegions())
				{
					if (h->name() == name)
					{
						region = h;
						break;
					}
				}
				
				if (region != nullptr)
				{
					dataModel()->removeFieldRegion(region);
					field_->removeHighlight(region);
				}
			}

			void ZebraViewWidget::addHighlight(Alliance a, HighlightType ht)
			{
				QPointF fpt = field_->globalToWorld(menu_point_);

				if (ht == HighlightType::Circle)
				{
					field_->selectCircularArea();
					auto cb = std::bind(&ZebraViewWidget::areaSelected, this, std::placeholders::_1, a, ht);
					field_connect_ = connect(field_, &PathFieldView::areaSelected, cb);
				}
				else if (ht == HighlightType::Rectangle)
				{
					field_->selectRectArea();
					auto cb = std::bind(&ZebraViewWidget::areaSelected, this, std::placeholders::_1, a, ht);
					field_connect_ = connect(field_, &PathFieldView::areaSelected, cb);
				}
				else if (ht == HighlightType::Polygon)
				{
					field_->selectPolygonArea();
					auto cb = std::bind(&ZebraViewWidget::polygonSelected, this, std::placeholders::_1, a);
					field_connect_ = connect(field_, &PathFieldView::polySelected, cb);
				}
			}

			void ZebraViewWidget::sliderChangedAnimationState(bool state, double mult)
			{
				animation_mult_ = mult;

				if (state)
				{
					if (animation_timer_ == nullptr)
					{
						animationSetTime(slider_->rangeStart());

						animation_timer_ = new QTimer(this);
						animation_timer_->setInterval(std::chrono::milliseconds(static_cast<int>(AnimationTimeStep * 1000)));
						(void)connect(animation_timer_, &QTimer::timeout, this, &ZebraViewWidget::animationProc);
						animation_timer_->start();
					}
				}
				else
				{
					if (animation_timer_ != nullptr)
					{
						animation_timer_->stop();
						delete animation_timer_;
						animation_timer_ = nullptr;
					}
				}
			}

			void ZebraViewWidget::animationSetTime(double t)
			{
				animation_time_ = t;
				for (TrackEntry &te : entries_) {
					te.track()->setCurrentTime(t);
				}
				slider_->setCurrentTime(t);

				slider_->update();
				field_->update();
			}

			void ZebraViewWidget::animationProc()
			{
				animation_time_ += animation_mult_ * AnimationTimeStep ;
				if (animation_time_ > slider_->rangeEnd())
					animation_time_ = slider_->rangeStart();

				animationSetTime(animation_time_);
			}

			void ZebraViewWidget::showEvent(QShowEvent *ev)
			{
				if (mode_ == Mode::Uninitialized)
				{
					//
					// Normally changing mode select current item would trigger a signal which would
					// recreate the plot.  But when we check matches below, we will do the same.  So, to prevent
					// going through the process of creating the plot twice during initialization, we block the 
					// signals while we setup the view mode
					//
					mode_select_->blockSignals(true);
					mode_select_->setCurrentIndex(0);
					mode_select_->blockSignals(false);

					matches_->setChecked(true);
					matchesSelected(true);
				}
			}

			void ZebraViewWidget::checkChanged(int state)
			{
				static bool inside = false;

				if (inside)
					return;

				inside = true;
				Qt::CheckState st = static_cast<Qt::CheckState>(state);
				if (st != Qt::CheckState::PartiallyChecked)
				{
					for (int i = 0; i < list_->count(); i++)
					{
						auto item = list_->item(i);
						item->setCheckState(st);
					}
				}
				inside = false;
			}

			void ZebraViewWidget::detailItemChanged(QListWidgetItem* item)
			{
				int entindex = item->data(Qt::UserRole).toInt();
				assert(entindex >= 0 && entindex < entries_.size());

				if (item->checkState() == Qt::CheckState::Checked)
				{
					if (!entries_[entindex].enabled())
					{
						field_->addTrack(entries_[entindex].track());
						entries_[entindex].setEnabled(true);
					}
				}
				else if (item->checkState() == Qt::CheckState::Unchecked)
				{
					if (entries_[entindex].enabled())
					{
						field_->removeTrack(entries_[entindex].track());
						entries_[entindex].setEnabled(false);
					}
				}
				field_->update();

				bool allon = true;
				bool alloff = true;

				for (int i = 0; i < list_->count(); i++)
				{
					Qt::CheckState st = list_->item(i)->checkState();
					if (st == Qt::CheckState::Checked)
						alloff = false;
					else if (st == Qt::CheckState::Unchecked)
						allon = false;
				}

				all_->blockSignals(true);
				if (allon)
				{
					all_->setCheckState(Qt::CheckState::Checked);
				}
				else if (alloff)
				{
					all_->setCheckState(Qt::CheckState::Unchecked);
				}
				else
				{
					all_->setCheckState(Qt::CheckState::PartiallyChecked);
				}
				all_->blockSignals(false);
			}

			void ZebraViewWidget::rangeChanged(double minv, double maxv)
			{
				for (auto t : field_->tracks())
				{
					t->setRange(slider_->rangeStart(), slider_->rangeEnd());
				}

				updatePerformance(true);
				field_->update();
			}

			void ZebraViewWidget::clearView()
			{
				box_->clear();
				field_->clearTracks();
				entries_.clear();
			}

			void ZebraViewWidget::refreshView()
			{
				if (mode_ != Mode::Uninitialized)
					createPlot();
			}

			QColor ZebraViewWidget::matchRobotColor(xero::scouting::datamodel::Alliance c, int slot)
			{
				QColor ret(0, 0, 0, 255);

				if (c == Alliance::Red)
				{
					switch (slot)
					{
					case 1:
						ret = QColor(255, 0, 0, 255);
						break;
					case 2:
						ret = QColor(128, 0, 0, 255);
						break;
					case 3:
						ret = QColor(255, 128, 128, 255);
						break;
					}
				}
				else
				{
					switch (slot)
					{
					case 1:
						ret = QColor(0, 0, 255, 255);
						break;
					case 2:
						ret = QColor(128, 128, 255, 255);
						break;
					case 3:
						ret = QColor(50, 168, 158, 255);
						break;
					}
				}

				return ret;
			}

			void ZebraViewWidget::updateComboBoxMatch()
			{
				if (dataModel() != nullptr)
				{
					box_->blockSignals(true);
					box_->clear();

					for (auto m : dataModel()->matches())
					{
						if (!m->zebra().isEmpty())
							box_->addItem(m->title(), m->key());
					}
					box_->blockSignals(false);
					box_->setCurrentIndex(0);
				}
				else
				{
					box_->clear();
				}
			}

			void ZebraViewWidget::updateComboBoxTeam()
			{

				if (dataModel() != nullptr)
				{
					box_->blockSignals(true);
					box_->clear();
					auto dm = dataModel();
					if (dm != nullptr) {
						std::list<std::shared_ptr<const DataModelTeam>> teams = dm->teams();
						teams.sort([](std::shared_ptr<const DataModelTeam> a, std::shared_ptr<const DataModelTeam> b) -> bool
							{
								return a->number() < b->number();
							});

						for (auto t : teams)
							box_->addItem(QString::number(t->number()) + " - " + t->nick(), t->key());
					}
					box_->blockSignals(false);
					box_->setCurrentIndex(0);
				}
				else
				{
					box_->clear();
				}
			}


			//
			// This is called when the matches radio button changes.  We only process the
			// event when checked is true meaning the user has asked for matches
			//
			void ZebraViewWidget::matchesSelected(bool checked)
			{
				if (!checked)
					return;

				mode_ = Mode::SingleMatch;
				setNeedRefresh();
				updateComboBoxMatch();
				createPlot();
			}

			//
			// This is called when the robot radio button changes.  We only process the
			// event when checked is true meaning the user has asked for robots
			//
			void ZebraViewWidget::robotSelected(bool checked)
			{
				if (!checked)
					return;

				mode_ = Mode::SingleTeam;
				setNeedRefresh();
				updateComboBoxTeam();
				createPlot();
			}

			void ZebraViewWidget::comboxChanged(int which)
			{
				if (animation_timer_ != nullptr)
				{
					animationSetTime(slider_->rangeStart());
				}
				setNeedRefresh();
				createPlot();
				field_->update();
			}

			void ZebraViewWidget::modeChanged(int which)
			{
				auto m = static_cast<PathFieldView::ViewMode>(mode_select_->itemData(which).toInt());
				field_->setViewMode(m);
				field_->update();

				if (m == PathFieldView::ViewMode::Robot)
					slider_->setRangeMode(false);
				else
					slider_->setRangeMode(true);
			}

			void ZebraViewWidget::getTimes(const QJsonArray& array, std::shared_ptr<RobotTrack> track)
			{
				for (int i = 0; i < array.size(); i++)
				{
					if (array[i].isDouble())
					{
						double d = array[i].toDouble();
						track->addTime(d);
					}
				}
			}

			bool ZebraViewWidget::extractOneAlliance(const QJsonArray& arr, int index, std::shared_ptr<xero::scouting::datamodel::RobotTrack> track)
			{
				if (!arr[index].isObject())
					return false;

				const QJsonObject& obj = arr[index].toObject();

				if (!obj.contains("xs") || !obj.value("xs").isArray())
					return false;

				if (!obj.contains("ys") || !obj.value("ys").isArray())
					return false;

				const QJsonArray& xa = obj.value("xs").toArray();
				const QJsonArray& ya = obj.value("ys").toArray();

				if (xa.size() != ya.size())
					return false;

				for (int k = 0; k < xa.size(); k++)
				{
					if (xa[k].isUndefined() || ya[k].isUndefined())
						continue;

					if (xa[k].isNull() || ya[k].isNull())
						continue;

					if (!xa[k].isDouble() || !ya[k].isDouble())
						continue;

					double xv = xa[k].toDouble();
					double yv = ya[k].toDouble();

					track->addPoint(QPointF(xv, yv));
				}
				return true;
			}

			//
			// Called to create a plot from the ground up with everything visible
			//
			void ZebraViewWidget::createPlot()
			{
				QVariant v = box_->itemData(box_->currentIndex());
				if (mode_ == Mode::SingleMatch)
				{
					createPlotMatch(v.toString());
				}
				else
				{
					createPlotTeam(v.toString());
				}

				all_->blockSignals(true);
				all_->setCheckState(Qt::CheckState::Checked);
				all_->blockSignals(false);
			}

			//
			// Create a new track object
			//
			std::shared_ptr<RobotTrack> ZebraViewWidget::createTrack(const QString& mkey, const QString& tkey)
			{
				Alliance c;
				int slot;
				QString color;

				auto m = dataModel()->findMatchByKey(mkey);
				if (m == nullptr)
					return nullptr;

				if (!m->teamToAllianceSlot(tkey, c, slot))
					return nullptr;

				auto team = dataModel()->findTeamByKey(tkey);
				if (team == nullptr)
					return nullptr;

				color = toString(c);

				if (!m->hasZebra())
					return nullptr;

				const QJsonObject& obj = m->zebra();
				if (!obj.contains("times") || !obj.value("times").isArray())
					return nullptr;

				if (!obj.contains("alliances") || !obj.value("alliances").isObject())
					return nullptr;

				QJsonObject aobj = obj.value("alliances").toObject();

				if (!aobj.contains(color) || !aobj.value(color).isArray())
					return nullptr;

				QJsonArray array = aobj.value(color).toArray();
				if (array.size() != 3)
					return nullptr;

				QString title;
				if (mode_ == Mode::SingleMatch)
				{
					title = QString::number(team->number());
				}
				else
				{
					c = Alliance::Red;
					title = m->title(true);
				}

				auto t = std::make_shared<RobotTrack>(title, matchRobotColor(c, slot), c);
				t->setRange(slider_->rangeStart(), slider_->rangeEnd());

				if (!extractOneAlliance(array, slot - 1, t))
					return nullptr;

				QJsonArray tarray = obj.value("times").toArray();
				getTimes(tarray, t);
				
				TrackEntry te(mkey, tkey, t);
				entries_.push_back(te);

				if (c == Alliance::Blue && mode_ == Mode::SingleTeam)
					t->transform(54.0, 27.0);

				field_->addTrack(t);

				return t;
			}

			//
			// Called when we want to create a new zebra plot based on the a given match
			//
			void ZebraViewWidget::createPlotMatch(const QString& mkey)
			{
				if (!needsRefresh())
					return;

				field_->clearTracks();
				entries_.clear();

				auto m = dataModel()->findMatchByKey(mkey);
				if (m == nullptr)
					return;

				Alliance c = Alliance::Red;
				for (int i = 1; i <= 3; i++)
				{
					QString tkey = m->team(c, i);
					createTrack(mkey, tkey);
				}
				c = Alliance::Blue;
				for (int i = 1; i <= 3; i++)
				{
					QString tkey = m->team(c, i);
					createTrack(mkey, tkey);
				}

				updateDisplay();
			}

			void ZebraViewWidget::createPlotTeam(const QString& tkey)
			{
				if (!needsRefresh())
					return;

				entries_.clear();
				field_->clearTracks();

				for (auto m : dataModel()->matches())
				{
					if (!m->teamIsInAlliance(tkey))
						continue;

					createTrack(m->key(), tkey);
				}

				updateDisplay();
			}

			void ZebraViewWidget::updateDisplay()
			{
				updateDetail();
				updatePerformance(false);
				updateSlider();
			}

			void ZebraViewWidget::updateDetail()
			{
				list_->clear();

				for (int i = 0; i < entries_.size(); i++)
				{
					const TrackEntry& entry = entries_[i];
					QListWidgetItem* item;

					if (mode_ == Mode::SingleMatch)
					{
						auto team = dataModel()->findTeamByKey(entry.teamKey());
						item = new QListWidgetItem(QString::number(team->number()) + " - " + team->nick());
					}
					else
					{
						auto m = dataModel()->findMatchByKey(entry.matchKey());
						item = new QListWidgetItem(m->title());
					}
					item->setData(Qt::UserRole, QVariant(i));

					if (entry.track() != nullptr)
						item->setCheckState(Qt::CheckState::Checked);
					else
						item->setCheckState(Qt::CheckState::Unchecked);

					list_->addItem(item);
				}
			}

			void ZebraViewWidget::updatePerformance(bool flush)
			{
				info_->clear();
				for (TrackEntry& te : entries_)
				{
					auto t = dataModel()->findTeamByKey(te.teamKey());
					auto m = dataModel()->findMatchByKey(te.matchKey());

					QTreeWidgetItem* item = new QTreeWidgetItem(info_);
					if (mode_ == Mode::SingleMatch)
					{
						item->setText(0, t->title());
					}
					else
					{
						item->setText(0, m->title());
					}

					if (flush)
						te.info().flush();

					QString tt;
					ScoutingDataMapPtr data = te.info().characterize();
					QTreeWidgetItem* info;
					for (auto pair : *data)
					{
						info = new QTreeWidgetItem(item);
						QString value = QString::number(pair.second.toDouble(), 'f', 2);
						QString entry = pair.first + " = " + value;
						info->setText(0, entry);

						if (tt.length() > 0)
							tt += "\n";
						tt += entry;
					}

					item->setToolTip(0, tt);
				}

				info_->resizeColumnToContents(0);
				info_->resizeColumnToContents(1);
			}

			void ZebraViewWidget::updateSlider()
			{
				double minv = std::numeric_limits<double>::max();
				double maxv = std::numeric_limits<double>::min();

				for (const TrackEntry& te : entries_)
				{
					double mymin = te.track()->time(0);
					if (mymin < minv)
						minv = mymin;

					double mymax = te.track()->time(te.track()->timeCount() - 1);
					if (mymax > maxv)
						maxv = mymax;
				}

				slider_->setMinimum(minv);
				slider_->setMaximum(maxv);
			}
		}
	}
}

