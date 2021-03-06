#include "World.h"
#include "..\base\Assert.h"
#include "actions\ScalingAction.h"
#include "actions\RemoveAfterAction.h"
#include "actions\ScaleByPathAction.h"
#include "actions\MoveByAction.h"
#include "actions\MoveToAction.h"
#include "actions\RotateByAction.h"
#include "actions\RotateAction.h"
#include "actions\RotateToTargetAction.h"
#include "actions\LookAtAction.h"
#include "actions\SeekAction.h"
#include "actions\SeparateAction.h"
#include "..\profiler\Profiler.h"
#include "actions\CollisionAction.h"
#include "actions\AlphaFadeToAction.h"
#include "actions\ScaleAxesAction.h"
#include "actions\ColorFlashAction.h"
#include "actions\WiggleAction.h"
#include "actions\AlignToForceAction.h"

namespace ds {

	World::World() : _boundingRect(0,0,1024,768) {
		_data = new ChannelArray;
		int sizes[] = { sizeof(v3), sizeof(v3), sizeof(v3) ,sizeof(Texture) , sizeof(Color), sizeof(float), sizeof(int), sizeof(v3),sizeof(int),sizeof(StaticHash)};
		_data->init(sizes, 10);
		_templates = 0;
		_actionManager = new ActionManager(_data,_boundingRect);
		_behaviors = new Behaviors(_actionManager);
	}


	World::~World()	{
		delete _behaviors;
		delete _actionManager;		
		delete _data;
	}

	// -----------------------------------------------
	// set world dimension
	// -----------------------------------------------
	void World::setWorldDimension(const v2& dim) {
		_boundingRect = Rect(0.0f, 0.0f, dim.x, dim.y);
	}

	void World::setBoundingRect(const Rect& r) {
		_boundingRect = r;
		_actionManager->setBoundingRect(r);
	}

	void World::useTemplates(WorldEntityTemplates* templates) {
		_templates = templates;
	}
	// -----------------------------------------------
	// create
	// -----------------------------------------------
	ID World::create() {
		return _data->add();
	}

	// -----------------------------------------------
	// contains
	// -----------------------------------------------
	bool World::contains(ID id) const {
		return _data->contains(id);
	}

	// -----------------------------------------------
	// create
	// -----------------------------------------------
	ID World::create(const v2& pos, const Texture& texture, int type, float rotation, const v2& scale, const Color& color) {
		ID id = _data->add();
		//LOGC("world") << "create - id: " << id;
		_data->set<v3>(id, WEC_POSITION, v3(pos));
		_data->set<Texture>(id, WEC_TEXTURE, texture);
		_data->set<v3>(id, WEC_ROTATION, v3(rotation,0.0f,0.0f));
		_data->set<v3>(id, WEC_SCALE, v3(scale.x, scale.y, 1.0f));
		_data->set<Color>(id, WEC_COLOR, color);
		_data->set<int>(id, WEC_TYPE, type);
		_data->set<v3>(id, WEC_FORCE, v3(0.0f));
		_data->set<int>(id, WEC_NAME, -1);
		_data->set<StaticHash>(id, WEC_HASH, SID("-"));
		return id;
	}

	ID World::create(const v2& pos, StaticHash entityHash) {
		assert(_templates != 0);
		int eidx = _templates->findIndex(entityHash);
		assert(eidx != -1);
		const WorldEntity& tpl = _templates->getTemplate(eidx);
		return create(pos, tpl.texture, tpl.type, tpl.rotation.x, tpl.scale.xy(), tpl.color);
	}

	uint32_t World::size() const {
		return _data->size;
	}

	void World::setRotation(ID id, const v3& rotation) {
		_data->set<v3>(id, WEC_ROTATION, rotation);
	}

	void World::setRotation(ID id, float rotation) {
		_data->set<v3>(id, WEC_ROTATION, v3(rotation));
	}

	void World::setColor(ID id, const Color& color) {
		_data->set<Color>(id, WEC_COLOR, color);
	}

	void World::setPosition(ID id, const v2& pos) {
		_data->set<v3>(id, WEC_POSITION, v3(pos));
	}

	void World::setPosition(ID id, const v3& pos) {
		_data->set<v3>(id, WEC_POSITION, pos);
	}

	const v3& World::getPosition(ID id) const {
		return _data->get<v3>(id, WEC_POSITION);
	}

	int World::getType(ID id) const {
		return _data->get<int>(id, WEC_TYPE);
	}

	const v3& World::getRotation(ID id) const {
		return _data->get<v3>(id, WEC_ROTATION);
	}

	const v3& World::getScale(ID id) const {
		return _data->get<v3>(id, WEC_SCALE);
	}

	void World::setTexture(ID id, const Texture& texture) {
		_data->set<Texture>(id, WEC_TEXTURE, texture);
	}

	void World::setScale(ID id, const v3& s) {
		_data->set<v3>(id, WEC_SCALE, s);
	}
	// -----------------------------------------------
	// scale by path
	// -----------------------------------------------
	void World::scaleByPath(ID id, V3Path* path, float ttl) {
		ScaleByPathAction* action = (ScaleByPathAction*)_actionManager->get(AT_SCALE_BY_PATH);
		action->attach(id, path, ttl);
	}

	void World::scaleAxes(ID id, int axes, float start, float end, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		ScaleAxesAction* action = (ScaleAxesAction*)_actionManager->get(AT_SCALE_AXES);
		action->attach(id, axes, start, end, ttl, mode, tweeningType);
	}
	// -----------------------------------------------
	// scale
	// -----------------------------------------------
	void World::scale(ID id, const v3& start, const v3& end, float ttl, int mode, const tweening::TweeningType& tweeningType) {		
		ScalingAction* action = (ScalingAction*)_actionManager->get(AT_SCALE);
		action->attach(id, WEC_SCALE, start, end, ttl, mode, tweeningType);
	}

	// -----------------------------------------------
	// remove
	// -----------------------------------------------
	void World::remove(ID id) {
		//LOGC("world") << "removing: " << id;
		if (_data->contains(id)) {
			_data->remove(id);
		}
		else {
			LOGE << "requesting to remove " << id << " but it is not part of the world";
		}
		_actionManager->removeByID(id);
		for (uint32_t i = 0; i < _customActions.size(); ++i) {
			_customActions[i]->removeByID(id);
		}
		if (_actionManager->supportCollisions()) {
			CollisionAction* collisionAction = _actionManager->getCollisionAction();
			collisionAction->removeByID(id);
		}
		_additionalData.remove(id);			
	}

	// -----------------------------------------------
	// remove all by type
	// -----------------------------------------------
	void World::removeByType(int type) {
		ID ids[1024];
		int num = find_by_type(type, ids, 1024);
		for (int i = 0; i < num; ++i) {
			remove(ids[i]);
		}		
	}

	// -----------------------------------------------
	// find by name
	// -----------------------------------------------
	ID World::findByName(StaticHash hash) {
		int* indices = _data->_sparse;
		for (int i = 0; i < _data->capacity; ++i) {
			if (indices[i] != -1) {
				StaticHash current = _data->get<StaticHash>(i, WEC_HASH);
				if (hash == current) {
					return i;
				}
			}
		}
		return INVALID_ID;
	}

	// -----------------------------------------------
	// find by type
	// -----------------------------------------------
	int World::find_by_type(int type, ID* ids, int max) const {
		int* indices = _data->_sparse;
		int cnt = 0;
		for (int i = 0; i < _data->capacity; ++i) {
			if (indices[i] != -1 && cnt < max) {
				int t = _data->get<int>(i, WEC_TYPE);
				if (t == type) {
					ids[cnt++] = i;
				}
			}
		}
		return cnt;
	}

	// -----------------------------------------------
	// attach name
	// -----------------------------------------------
	void World::attachName(ID id, const char* name) {
		int idx = gStringBuffer->append(name);
		_data->set<int>(id, WEC_NAME, idx);
		StaticHash hash = SID(name);
		_data->set<StaticHash>(id, WEC_HASH, hash);
		LOG << "id: " << id << " name: " << name << " hash: " << hash.get();
	}

	// -----------------------------------------------
	// attach collider
	// -----------------------------------------------
	void World::attachCollider(ID id, ShapeType type) {
		CollisionAction* collisionAction = _actionManager->getCollisionAction();
		const Texture& t = _data->get<Texture>(id, WEC_TEXTURE);
		v3 extent = v3(t.dim.x, t.dim.y,0.0f);
		collisionAction->attach(id, type, extent);
	}

	// -----------------------------------------------
	// attach collider
	// -----------------------------------------------
	void World::attachCollider(ID id, ShapeType type, const v2& extent) {
		CollisionAction* collisionAction = _actionManager->getCollisionAction();
		collisionAction->attach(id, type, v3(extent));
	}

	void World::ignoreCollisions(int firstType, int secondType) {
		CollisionAction* collisionAction = _actionManager->getCollisionAction();
		collisionAction->ignore(firstType, secondType);
	}

	// -----------------------------------------------
	// has collisions
	// -----------------------------------------------
	bool World::hasCollisions() const {
		CollisionAction* collisionAction = _actionManager->getCollisionAction();
		return collisionAction->hasCollisions();
	}

	// -----------------------------------------------
	// get collision
	// -----------------------------------------------
	const Collision& World::getCollision(int idx) const {
		CollisionAction* collisionAction = _actionManager->getCollisionAction();
		return collisionAction->getCollision(idx);
	}

	// -----------------------------------------------
	// num collisions
	// -----------------------------------------------
	uint32_t World::numCollisions() const {
		CollisionAction* collisionAction = _actionManager->getCollisionAction();
		//return _collisionAction->numCollisions();
		return collisionAction->numCollisions();
	}

	// -----------------------------------------------
	// move by (2D)
	// -----------------------------------------------
	void World::moveBy(ID id, const v2& velocity, float ttl, bool bounce) {
		moveBy(id, v3(velocity), ttl, bounce);
	}

	// -----------------------------------------------
	// move by
	// -----------------------------------------------
	void World::moveBy(ID id, const v3& velocity, float ttl, bool bounce) {
		MoveByAction* action = (MoveByAction*)_actionManager->get(AT_MOVE_BY);
		action->attach(id, velocity, ttl, bounce);
	}

	void World::moveTo(ID id, const v3& start, const v3& end, float ttl, const tweening::TweeningType& type) {
		MoveToAction* action = (MoveToAction*)_actionManager->get(AT_MOVE_TO);
		action->attach(id, start, end, ttl, type);
	}

	// -----------------------------------------------
	// remove after
	// -----------------------------------------------
	void World::removeAfter(ID id, float ttl) {		
		RemoveAfterAction* action = (RemoveAfterAction*)_actionManager->get(AT_REMOVE_AFTER);
		action->attach(id, ttl);
	}

	void World::flashColor(ID id, const Color& startColor, const Color& endColor, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		ColorFlashAction* action = (ColorFlashAction*)_actionManager->get(AT_COLOR_FLASH);
		action->attach(id, startColor, endColor, ttl, mode, tweeningType);
	}

	void World::alphaFadeTo(ID id, float start, float end, float ttl) {
		AlphaFadeToAction* action = (AlphaFadeToAction*)_actionManager->get(AT_ALPHA_FADE_TO);
		action->attach(id, start, end, ttl);
	}
	// -----------------------------------------------
	// separate
	// -----------------------------------------------
	void World::separate(ID id, int type, float minDistance, float relaxation) {
		SeparateAction* action = (SeparateAction*)_actionManager->get(AT_SEPARATE);
		action->attach(id, type, minDistance, relaxation);
	}

	// -----------------------------------------------
	// seek
	// -----------------------------------------------
	void World::seek(ID id, ID target, float velocity) {
		SeekAction* action = (SeekAction*)_actionManager->get(AT_SEEK);
		action->attach(id, target, velocity);
	}

	// -----------------------------------------------
	// look at
	// -----------------------------------------------
	void World::lookAt(ID id, ID target, float ttl) {
		LookAtAction* action = (LookAtAction*)_actionManager->get(AT_LOOK_AT);
		action->attach(id, target, ttl);
	}

	// -----------------------------------------------
	// rotate to
	// -----------------------------------------------
	void World::rotateTo(ID id, ID target, float angleVelocity) {
		RotateToTargetAction* action = (RotateToTargetAction*)_actionManager->get(AT_ROTATE_TO_TARGET);
		action->attach(id, target, angleVelocity);
	}

	void World::rotate(ID id, const v3& velocity, float ttl) {
		RotateAction* action = (RotateAction*)_actionManager->get(AT_ROTATE);
		action->attach(id, velocity, ttl);
	}
	// -----------------------------------------------
	// rotate by
	// -----------------------------------------------
	void World::rotateBy(ID id, float angle, float ttl) {
		RotateByAction* action = (RotateByAction*)_actionManager->get(AT_ROTATE_BY);
		action->attach(id, angle, ttl);
	}

	// -----------------------------------------------
	// stop action
	// -----------------------------------------------
	void World::stopAction(ID id, ActionType type) {
		_actionManager->stopAction(id, type);
	}

	// -----------------------------------------------
	// is active
	// -----------------------------------------------
	bool World::isActive(ID id, ActionType type) {
		return _actionManager->isActive(id, type);
	}

	// -----------------------------------------------
	// tick
	// -----------------------------------------------
	void World::tick(float dt) {
		ZoneTracker m("World::tick");
		_buffer.reset();
		// reset forces
		v3* forces = (v3*)_data->get_ptr(WEC_FORCE);		
		for (uint32_t i = 0; i < _data->size; ++i) {
			*forces = v3(0.0f);
			++forces;
		}

		_actionManager->update(dt, _buffer);
		
		// update all custom actions
		{
			ZoneTracker u2("World::tick::updateCustom");
			for (uint32_t i = 0; i < _customActions.size(); ++i) {
				_customActions[i]->update(dt, _buffer);
			}
		}
		

		_behaviors->tick(dt);

		// apply forces
		{
			ZoneTracker af("World::tick::applyForces");
			forces = (v3*)_data->get_ptr(WEC_FORCE);
			v3* positions = (v3*)_data->get_ptr(WEC_POSITION);
			for (uint32_t i = 0; i < _data->size; ++i) {
				*positions += *forces;
				++forces;
				++positions;
			}
		}
		// handle collisions
		{
			ZoneTracker cl("World::tick::collisions");
			// do not create collision action
			if (_actionManager->supportCollisions()) {
				CollisionAction* collisionAction = _actionManager->getCollisionAction();
				collisionAction->update(dt, _buffer);
			}
		}		

		// process events / kill entities
		{
			ZoneTracker ev("World::tick::events");
			for (uint32_t i = 0; i < _buffer.events.size(); ++i) {
				const ActionEvent& e = _buffer.events[i];
				if (e.action == AT_KILL) {
					remove(e.id);
				}
				_behaviors->processEvent(e);
			}
		}
	}

	void World::generateJSON(std::string& resp) {
		int* indices = _data->_sparse;
		resp.append("[\n");
		bool started = true;
		for (int i = 0; i < _data->capacity; ++i) {
			if (indices[i] != -1) {
				if (started) {
					started = false;
				}
				else {
					resp.append(" , \n");
				}
				resp.append("{\n");
				resp.append("\"ID\" : \"1\",\n");
				resp.append("\"Index\" : \"1\",\n");
				resp.append("\"Position\" : \"1\",\n");
				resp.append("\"Texture\" : \"1\",\n");
				resp.append("\"Rotation\" : \"1\",\n");
				resp.append("\"Scale\" : \"1\",\n");
				resp.append("\"Color\" : \"1\",\n");
				resp.append("\"Type\" : \"1\",\n");
				resp.append("\"Force\" : \"1\"\n");
				resp.append("}\n");
			}
		}
		resp.append("]\n");
	}
	// -----------------------------------------------
	// save report
	// -----------------------------------------------
	void World::saveReport(const ds::ReportWriter& writer) {
		writer.startBox("World");
		const char* OVERVIEW_HEADERS[] = { "ID", "Index", "Position", "Texture", "Rotation", "Scale", "Color", "Type", "Force","Name","Hash" };
		writer.startTable(OVERVIEW_HEADERS, 11);
		int* indices = _data->_sparse;
		for (int i = 0; i < _data->capacity; ++i) {
			if (indices[i] != -1) {
				writer.startRow();
				writer.addCell(i);
				writer.addCell(indices[i]);
				writer.addCell(_data->get<v3>(i, WEC_POSITION));
				writer.addCell(_data->get<Texture>(i, WEC_TEXTURE));
				writer.addCell(RADTODEG(_data->get<v3>(i, WEC_ROTATION).x));
				writer.addCell(_data->get<v3>(i, WEC_SCALE));
				writer.addCell(_data->get<Color>(i, WEC_COLOR));
				writer.addCell(_data->get<int>(i, WEC_TYPE));
				writer.addCell(_data->get<v3>(i, WEC_FORCE));
				int idx = _data->get<int>(i, WEC_NAME);
				if (idx != -1) {
					writer.addCell(gStringBuffer->get(idx));
				}
				else {
					writer.addCell("-");
				}
				writer.addCell(_data->get<StaticHash>(i, WEC_HASH).get());
				writer.endRow();
			}
		}
		writer.endTable();
		writer.endBox();
		_additionalData.save(writer);
		_actionManager->saveReport(writer);
		for (uint32_t i = 0; i < _customActions.size(); ++i) {
			_customActions[i]->saveReport(writer);
		}
		if (_actionManager->supportCollisions()) {
			CollisionAction* collisionAction = _actionManager->getCollisionAction();
			collisionAction->saveReport(writer);
		}
	}

	// -----------------------------------------------
	// create behavior
	// -----------------------------------------------
	ID World::createBehavior(const char* name) {
		return _behaviors->create(name);		
	}

	ID World::createBehavior(const char* name, ds::ActionDefinition* definitions, int num) {
		ID ret = createBehavior(name);
		for (int i = 0; i < num; ++i) {
			_behaviors->addSettings(ret, definitions[i].settings);
		}
		return ret;
	}

	// -----------------------------------------------
	// add array of settings to behavior
	// -----------------------------------------------
	void World::addSettings(SettingsDefinition* definitions, int num) {
		for (int i = 0; i < num; ++i) {
			_behaviors->addSettings(definitions[i].behavior, definitions[i].settings);
		}
	}

	// -----------------------------------------------
	// add settings to behavior
	// -----------------------------------------------
	void World::addSettings(ID behaviorID, ActionSettings* settings) {
		_behaviors->addSettings(behaviorID, settings);
	}

	// -----------------------------------------------
	// start behavior
	// -----------------------------------------------
	void World::startBehavior(const StaticHash& hash, ID id) {
		_behaviors->start(hash, id);		
	}

	// -----------------------------------------------
	// connect behaviors
	// -----------------------------------------------
	void World::connectBehaviors(StaticHash first, const ActionType& type, StaticHash second, int objectType) {
		_behaviors->connect(first, type, second, objectType);		
	}

	// -----------------------------------------------
	// connect behaviors
	// -----------------------------------------------
	void World::connectBehaviors(ID first, const ActionType& type, ID second, int objectType) {
		_behaviors->connect(first, type, second, objectType);		
	}

	// -----------------------------------------------
	// connect behaviors
	// -----------------------------------------------
	void World::connectBehaviors(ConnectionDefinition* definitions, int num, int objectType) {
		for (int i = 0; i < num; ++i) {
			_behaviors->connect(definitions[i].from, definitions[i].type, definitions[i].to, objectType);
		}
	}

}