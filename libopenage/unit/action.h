// Copyright 2014-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <memory>
#include <vector>

#include "../gamestate/old/resource.h"
#include "../pathfinding/path.h"
#include "attribute.h"
#include "research.h"
#include "unit.h"
#include "unit_container.h"

namespace openage {

class TerrainSearch;

// TODO use a type instead of unsigned int for time

/**
 * A interval triggering timer used in actions.
 * TODO find a better name for triggers
 */
class IntervalTimer {
public:
	/**
	 * Constructs a timer with a given interval
	 */
	IntervalTimer(unsigned int interval);

	/**
	 * Constructs a timer with a given interval which will
	 * stop after a given number of triggers.
	 */
	IntervalTimer(unsigned int interval, int max_triggers);

	void skip_to_trigger();

	bool update(unsigned int time);

	/**
	 * Returns the time until the next trigger
	 */
	unsigned int get_time_left() const;

	float get_progress() const;

	/**
	 * Returns true if at least one interval has passed.
	 */
	bool has_triggers() const;

	/**
	 * Returns true if the interval passed have reached the max.
	 */
	bool finished() const;

	/**
	 * Returns the number of intervals passed.
	 */
	int get_triggers() const {
		return this->triggers;
	}

	unsigned int get_interval() const {
		return this->interval;
	}

	void set_interval(unsigned int interval) {
		this->interval = interval;
	}

private:
	unsigned int interval;

	int max_triggers;

	unsigned int time_left;

	int triggers;
};


/**
 * Actions can be pushed onto any units action stack
 *
 * Each update cycle will perform the update function of the
 * action on top of this stack
 */
class UnitAction {
public:
	/**
	 * Require unit to be updated and an initial graphic type
	 */
	UnitAction(Unit *u, graphic_type initial_gt);

	virtual ~UnitAction() {}

	/**
	 * type of graphic this action should use
	 */
	graphic_type type() const;

	/**
	 * frame number to use on the current graphic
	 */
	float current_frame() const;

	/**
	 * each action has its own update functionality which gets called when this
	 * is the active action
	 */
	virtual void update(unsigned int time) = 0;

	/**
	 * action to perform when popped from a units action stack
	 */
	virtual void on_completion() = 0;

	/**
	 *	gets called for all actions on stack each update cycle
	 *	@return true when action is completed so it and everything above it can be popped
	 */
	virtual bool completed() const = 0;

	/**
	 * checks if the action can be interrupted, allowing it to be popped if the user
	 * specifies a new action, if false the action must reach a completed state
	 * before removal
	 * eg dead action must be completed and cannot be discarded
	 */
	virtual bool allow_interrupt() const = 0;

	/**
	 * control whether stack can discard the action automatically and
	 * should the stack be modifiable when this action is on top
	 *
	 * if true this action must complete and will not allow new actions
	 * to be pushed while it is active and also does not update the secondary actions
	 */
	virtual bool allow_control() const = 0;

	/**
	 * debug string to identify action types
	 */
	virtual std::string name() const = 0;

	/**
	 * determines which graphic should be used for drawing this unit
	 * finds the default graphic using the units type, used by most actions
	 *
	 * this virtual function is overriden for special cases such as
	 * villager task graphics
	 */
	virtual const graphic_set &current_graphics() const;

	void draw_debug(const LegacyEngine &engine);

	/**
	 * common functions for actions
	 */
	void face_towards(const coord::phys3 pos);

	/**
	 * Damage a unit, returns true if the unit was killed in the process
	 */
	bool damage_unit(Unit &target);

	void move_to(Unit &target, bool use_range = true);

	/**
	 * produce debug info such as visualising paths
	 */
	static bool show_debug;

	/**
	 * a small distance to which units are considered touching
	 * when within this distance
	 */
	static coord::phys_t adjacent_range(Unit *u);

	/**
	 * looks at an ranged attributes on the unit
	 * otherwise returns same as adjacent_range()
	 */
	static coord::phys_t get_attack_range(Unit *u);

	/**
	 * looks at heal attribute on the unit
	 * otherwise returns same as adjacent_range()
	 */
	static coord::phys_t get_heal_range(Unit *u);

protected:
	/**
	 * the entity being updated
	 */
	Unit *entity;

	/**
	 * common graphic controls
	 */
	graphic_type graphic;
	float frame;
	float frame_rate;

	/**
	 * additional drawing for debug purposes
	 */
	std::function<void(const LegacyEngine &)> debug_draw_action;
};

/**
 * Base class for actions which target another unit such as
 * gather, attack, heal and convert
 * TODO implement min range
 */
class TargetAction : public UnitAction {
public:
	/**
	 * action_rad is how close a unit must come to another
	 * unit to be considered to touch the other, for example in
	 * gathering resource and melee attack
	 */
	TargetAction(Unit *e, graphic_type gt, UnitReference r, coord::phys_t action_rad);

	/**
	 * this constructor uses the default action radius formula which will
	 * bring the object as near to the target as the pathing grid will allow.
	 */
	TargetAction(Unit *e, graphic_type gt, UnitReference r);
	virtual ~TargetAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return true;
	}
	bool allow_control() const override {
		return true;
	}
	virtual std::string name() const override = 0;

	/**
	 * Control units action when in range of the target
	 */
	virtual void update_in_range(unsigned int, Unit *) = 0;
	virtual void on_completion_in_range(int target_type) = 0;
	virtual bool completed_in_range(Unit *) const = 0;

	coord::phys_t distance_to_target();
	Unit *update_distance();

	UnitReference get_target() const;
	int get_target_type_id() const;

	/**
	 * changes target, ending action when new target is invalid
	 */
	void set_target(UnitReference new_target);

private:
	UnitReference target;
	int target_type_id;
	int repath_attempts;
	bool end_action;

	/**
	 * tracks distance to target from last update
	 */
	coord::phys_t dist_to_target, radius;
};

/**
 * plays a fixed number of frames for the units dying animation
 */
class DecayAction : public UnitAction {
public:
	DecayAction(Unit *e);
	virtual ~DecayAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return false;
	}
	bool allow_control() const override {
		return false;
	}
	std::string name() const override {
		return "decay";
	}

private:
	float end_frame;
};

/**
 * plays a fixed number of frames for the units dying animation
 */
class DeadAction : public UnitAction {
public:
	DeadAction(
		Unit *e,
		std::function<void()> on_complete = []() {});
	virtual ~DeadAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return false;
	}
	bool allow_control() const override {
		return false;
	}
	std::string name() const override {
		return "dead";
	}

private:
	float end_frame;
	std::function<void()> on_complete_func;
};

/**
 * places an idle action on the stack once building is complete
 */
class FoundationAction : public UnitAction {
public:
	FoundationAction(Unit *e, bool add_destuction = false);
	virtual ~FoundationAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return true;
	}
	bool allow_control() const override {
		return false;
	}
	std::string name() const override {
		return "foundation";
	}

private:
	bool add_destruct_effect, cancel;
};

/**
 * keeps an entity in a fixed position
 */
class IdleAction : public UnitAction {
public:
	IdleAction(Unit *e);
	virtual ~IdleAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return false;
	}
	bool allow_control() const override {
		return true;
	}
	std::string name() const override {
		return "idle";
	}

private:
	// look for auto task actions
	std::shared_ptr<TerrainSearch> search;
	ability_set auto_abilities;
};

/**
 * moves an entity to another location
 */
class MoveAction : public UnitAction {
public:
	/**
	 * moves unit to a given fixed location
	 */
	MoveAction(Unit *e, coord::phys3 tar, bool repath = true);

	/**
	 * moves a unit to within a distance to another unit
	 */
	MoveAction(Unit *e, UnitReference tar, coord::phys_t within_range);
	virtual ~MoveAction();

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return true;
	}
	bool allow_control() const override {
		return true;
	}
	std::string name() const override {
		return "move";
	}

	coord::phys3 next_waypoint() const;

private:
	UnitReference unit_target;
	coord::phys3 target;

	// how near the units should come to target
	coord::phys_t distance_to_target, radius;

	path::Path path;

	// should a new path be found if unit gets blocked
	bool allow_repath, end_action;

	void initialise();

	/**
	 * use a star to find a path to target
	 */
	void set_path();

	/**
	 * updates the distance_to_target value
	 */
	void set_distance();
};

/**
 * garrison inside a building
 */
class GarrisonAction : public TargetAction {
public:
	GarrisonAction(Unit *e, UnitReference build);
	virtual ~GarrisonAction() {}

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int) override {}
	bool completed_in_range(Unit *) const override {
		return this->complete;
	}
	std::string name() const override {
		return "garrison";
	}

private:
	bool complete;
};

/**
 * garrison inside a building
 */
class UngarrisonAction : public UnitAction {
public:
	UngarrisonAction(Unit *e, const coord::phys3 &pos);
	virtual ~UngarrisonAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override {
		return this->complete;
	}
	bool allow_interrupt() const override {
		return true;
	}
	bool allow_control() const override {
		return true;
	}
	std::string name() const override {
		return "ungarrison";
	}

private:
	coord::phys3 position;
	bool complete;
};

/**
 * trains a new unit
 */
class TrainAction : public UnitAction {
public:
	TrainAction(Unit *e, UnitType *pp);
	virtual ~TrainAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override {
		return this->complete;
	}
	bool allow_interrupt() const override {
		return false;
	}
	bool allow_control() const override {
		return true;
	}
	std::string name() const override {
		return "train";
	}

	float get_progress() const {
		return this->timer.get_progress();
	}

private:
	UnitType *trained;

	IntervalTimer timer;
	bool started;
	bool complete;
};

/**
 * trains a new unit
 */
class ResearchAction : public UnitAction {
public:
	ResearchAction(Unit *e, Research *research);
	virtual ~ResearchAction() {}

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override {
		return this->complete;
	}
	bool allow_interrupt() const override {
		return false;
	}
	bool allow_control() const override {
		return true;
	}
	std::string name() const override {
		return "train";
	}

	float get_progress() const {
		return this->timer.get_progress();
	}
	const ResearchType *get_research_type() const {
		return this->research->type;
	}

private:
	Research *research;

	IntervalTimer timer;
	bool complete;
};

/**
 * builds a building
 */
class BuildAction : public TargetAction {
public:
	BuildAction(Unit *e, UnitReference foundation);
	virtual ~BuildAction() {}

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int) override {}
	bool completed_in_range(Unit *) const override {
		return this->complete >= 1.0f;
	}
	void on_completion() override;
	std::string name() const override {
		return "build";
	}

	float get_progress() const {
		return this->complete;
	}

private:
	float complete, build_rate;
	static constexpr float search_tile_distance = 9.0f;
};

/**
 * repairs a unit
 */
class RepairAction : public TargetAction {
public:
	RepairAction(Unit *e, UnitReference tar);
	virtual ~RepairAction() {}

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int) override {}
	bool completed_in_range(Unit *) const override {
		return this->complete;
	}
	std::string name() const override {
		return "repair";
	}

private:
	/**
	 * stores the cost of the repair for 1hp
	 */
	ResourceBundle cost;

	IntervalTimer timer;
	bool complete;
};

/**
 * gathers resource from another object
 */
class GatherAction : public TargetAction {
public:
	GatherAction(Unit *e, UnitReference tar);
	virtual ~GatherAction();

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int target_type) override;
	bool completed_in_range(Unit *) const override {
		return this->complete;
	}
	std::string name() const override {
		return "gather";
	}

private:
	bool complete, target_resource;
	UnitReference target;
	gamedata::unit_classes resource_class;
	UnitReference nearest_dropsite(game_resource res_type);
};

/**
 * attacks another unit
 */
class AttackAction : public TargetAction {
public:
	AttackAction(Unit *e, UnitReference tar);
	virtual ~AttackAction();

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int) override {}
	bool completed_in_range(Unit *) const override;
	std::string name() const override {
		return "attack";
	}

private:
	IntervalTimer timer;

	/**
	 * use attack action
	 */
	void attack(Unit &target);

	/**
	 * add a projectile game object which moves towards the target
	 */
	void fire_projectile(const Attribute<attr_type::attack> &att, const coord::phys3 &target);
};

/**
 * heals another unit
 */
class HealAction : public TargetAction {
public:
	HealAction(Unit *e, UnitReference tar);
	virtual ~HealAction();

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int) override {}
	bool completed_in_range(Unit *) const override;
	std::string name() const override {
		return "heal";
	}

private:
	IntervalTimer timer;

	/**
	 * use heal action
	 */
	void heal(Unit &target);
};

/**
 * convert an object
 */
class ConvertAction : public TargetAction {
public:
	ConvertAction(Unit *e, UnitReference tar);
	virtual ~ConvertAction() {}

	void update_in_range(unsigned int time, Unit *target_unit) override;
	void on_completion_in_range(int) override {}
	bool completed_in_range(Unit *) const override {
		return this->complete >= 1.0f;
	}
	std::string name() const override {
		return "convert";
	}

private:
	float complete;
};

/**
 * moves object to fly in a parabolic shape
 */
class ProjectileAction : public UnitAction {
public:
	ProjectileAction(Unit *e, coord::phys3 target);
	virtual ~ProjectileAction();

	void update(unsigned int time) override;
	void on_completion() override;
	bool completed() const override;
	bool allow_interrupt() const override {
		return false;
	}
	bool allow_control() const override {
		return false;
	}
	std::string name() const override {
		return "projectile";
	}

private:
	double grav;
	bool has_hit;
};

} // namespace openage
