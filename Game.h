#pragma once

#include "Common.h"
#include "Entity.h"
#include "EntityManager.h"

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
	sf::RenderWindow	m_window;						// the window we will draw to
	EntityManager		m_entities;						// vector of entities to maintain
	sf::Font			m_font;							// the font we will use to draw
	sf::Text			m_text;							// the score text to be drawn to the screen
	PlayerConfig		m_player_config;		
	EnemyConfig			m_enemy_config;
	BulletConfig		m_bullet_config;
	int					m_score= 0;
	int					m_high_score= 0;
	int					m_last_enemy_spawn_time= 0;
	int					m_spawn_interval;
	int					m_last_tether_shot_time= 0;
	int					m_last_difficulty_increase= 0;
	int					m_current_frame= 0;
	bool				m_spawn_player_called;
	bool				m_paused= false;				// whether we update the game logic
	bool				m_running= true;				// whether the game is running
	sf::Texture			m_background_image;
	sf::Sprite			m_background;

	std::shared_ptr<Entity> m_player;

	sf::RectangleShape m_tether_meter_outline;
	sf::RectangleShape m_tether_meter_progress;

	void initialize_game(const std::string &config);	// initialize the GameState with a config file path
	void set_paused();									// pause the game

	void s_movement();									// System: Entity position / movement update
	void s_user_input();								// System: User Input
	void s_life_span();									// System: Lifespan
	void s_render();									// System: Render / Drawing
	void s_enemy_spawner();								// System: Spawns Enemies
	void s_collision();									// System: Collisions
	void s_debuffs();									// System: Debuff update
	void s_difficulty();								// System: increases the spawn rate of enemies

	void spawn_player();
	void spawn_enemy();
	void spawn_small_enemies(std::shared_ptr<Entity> entity);
	void spawn_bullet(std::shared_ptr<Entity> entity, const c_Vec2 &mousePos);
	void spawn_tether_shot(std::shared_ptr<Entity> entity, const c_Vec2 &mousePos);
	void spawn_tether_field(std::shared_ptr<Entity> entity);
	void update_tether_meter();

public:

	Game(const std::string &config);					// constructor, takes in game config

	void run();
};