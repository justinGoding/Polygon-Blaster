#include "Entity.h"

Entity::Entity(const size_t i, const enum e_Tag &t)
	: m_id(i), m_tag(t) {}

bool Entity::is_active() const
{
	return m_active;
}

const e_Tag &Entity::tag() const
{
	return m_tag;
}

const size_t Entity::id() const
{
	return m_id;
}

void Entity::destroy()
{
	m_active= false;
}