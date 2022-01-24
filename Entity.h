#pragma once

#include "Common.h"

#include "Components.h"
#include <memory>

enum class e_Tag{D, P, E, B, S, T, TF};

class Entity
{
	friend class EntityManager;

	bool		m_active= true;
	size_t		m_id= 0;
	e_Tag		m_tag= e_Tag::D;

	// constructor and destructor
	Entity(const size_t id, const enum e_Tag &tag);

public:

	// component pointers
	ptr<c_Transform>	c_Transform;
	ptr<c_Circle>		c_Circle;
	ptr<c_Rectangle>	c_Rectangle;
	ptr<c_Collision>	c_Collision;
	ptr<c_Input>		c_Input;
	ptr<c_Score>		c_Score;
	ptr<c_Lifespan>		c_Lifespan;
	ptr<c_Line>			c_Line;
	ptr<c_Tethered>		c_Tethered;
	ptr<c_Ethereal>		c_Ethereal;

	// private member access function
	bool is_active() const;
	const e_Tag &tag() const;
	const size_t id() const;
	void destroy();
};