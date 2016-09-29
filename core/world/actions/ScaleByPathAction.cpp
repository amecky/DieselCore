#include "ScaleByPathAction.h"
#include "..\..\math\GameMath.h"
#include "..\..\log\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ScaleByPathAction::ScaleByPathAction(ChannelArray* array) : AbstractAction(array, "scale_by_path") {
		int sizes[] = { sizeof(ID), sizeof(V3Path*), sizeof(float), sizeof(float) };
		_buffer.init(sizes, 4);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ScaleByPathAction::~ScaleByPathAction() {
		
	}

	void ScaleByPathAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (ID*)_buffer.get_ptr(0);
			_path = (V3Path**)_buffer.get_ptr(1);
			_timers = (float*)_buffer.get_ptr(2);
			_ttl = (float*)_buffer.get_ptr(3);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void ScaleByPathAction::attach(ID id, V3Path* path, float ttl) {
		int idx = create(id);
		_ids[idx] = id;
		_path[idx] = path;
		v3 s(1, 1, 1);
		path->get(0.0f, &s);
		_array->set<v3>(id,WEC_SCALE,s);
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;		
	}
	
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void ScaleByPathAction::update(float dt, ActionEventBuffer& buffer) {
		if ( _buffer.size > 0 ) {				
			// move
			v3 scale;
			for (int i = 0; i < _buffer.size; ++i) {
				float norm = math::norm(_timers[i], _ttl[i]);
				_path[i]->get(norm, &scale);
				_array->set<v3>(_ids[i], WEC_SCALE, scale);
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					_path[i]->get(1.0f, &scale);
					_array->set<v3>(_ids[i], WEC_SCALE, scale);
					int t = _array->get<int>(_ids[i], WEC_TYPE);
					buffer.add(_ids[i], AT_SCALE_BY_PATH, t);
					removeByIndex(i);
				}
			}
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void ScaleByPathAction::debug() {
		if ( _buffer.size > 0 ) {
			LOG << "---------- ScalingAction ---------- ";
		}
		for ( int i = 0; i < _buffer.size; ++i ) {
			LOG << i << " : id: " << _ids[i] << " timer: " << _timers[i];
		}
		/*
		std::map<SID,int>::iterator it = m_Mapping.begin();
		while ( it != m_Mapping.end()) {
			LOG << it->first << " = " << it->second;
			++it;
		}
		*/
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	/*
	void ScaleByPathAction::save(const ReportWriter& writer) {
		if (_buffer.size > 0) {
			writer.addSubHeader("ScalingAction");
			const char* HEADERS[] = { "Index", "ID", "Timer", "TTL" };
			writer.startTable(HEADERS, 4);
			for (int i = 0; i < _buffer.size; ++i) {
				writer.startRow();
				writer.addCell(i);
				writer.addCell(_ids[i]);
				writer.addCell(_timers[i]);
				writer.addCell(_ttl[i]);
				writer.endRow();
			}
			writer.endTable();
		}
	}
	*/

}