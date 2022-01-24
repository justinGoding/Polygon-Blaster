#include "EntityManager.h"

EntityManager::EntityManager() {}
	

void EntityManager::update()
{
	// add entities from m_entitiesToAdd the proper loactions
	//		- add them to the vector of all entities
	//		- add them to the vector inside the map, with the tag as a key
	for (auto &e : m_entities_to_add)
	{
		m_entities.push_back(e);
		m_entity_map[e->m_tag].push_back(e);
	}
	m_entities_to_add.clear();

	// remove dead entities from the vector of all entities
	remove_dead_entities(m_entities);

	// remove dead entities from each vector in the entity map
	// C++17 way of iterating through [key,value] pairs in a map
	for (auto &[tag, entityVec] : m_entity_map)
	{
		remove_dead_entities(entityVec);
	}
}

// iterates through a passed vector and erases any inactive entities
void EntityManager::remove_dead_entities(EntityVec &vec)
{
	auto v= vec.begin();
	while (v != vec.end())
	{
		if (!(*v)->is_active())
		{
			v= vec.erase(v);
		}
		else
		{
			v++;
		}
	}
}

// pushes an entity to the 'to_add' vector, which will be added to the entity vectors in the update() method
std::shared_ptr<Entity> EntityManager::add_entity(const enum e_Tag &tag)
{
	auto entity = std::shared_ptr<Entity>(new Entity(m_total_entities++, tag));

	m_entities_to_add.push_back(entity);
	
	return entity;
}

const EntityVec &EntityManager::get_entities()
{
	return m_entities;
}

const EntityVec &EntityManager::get_entities(const enum e_Tag &tag)
{
	return m_entity_map[tag];
}