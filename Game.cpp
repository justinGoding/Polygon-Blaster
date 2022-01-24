#include "Game.h"

Game::Game(const std::string &config)
{
	initialize_game(config);
}

void Game::initialize_game(const std::string &path)
{
	// Reads the config file for paramaters for the window,
	// font, player, enemies, and bullets.

	std::ifstream fin(path);

	std::string header,
		file_path;
	int	wWidth, wHeight, framerate, is_fullscreen,
		font_size, font_red, font_green, font_blue;

	while (fin >> header)
	{
		if (header == "Window")
		{
			fin >> wWidth >> wHeight >> framerate >> is_fullscreen;
		}
		else if (header == "Font")
		{
			fin >> file_path >> font_size >> font_red >> font_green >> font_blue;

			if (!m_font.loadFromFile(file_path))
			{
				std::cerr << "Could not load font!\n";
				exit(-1);
			}

			m_text.setFont(m_font);
			m_text.setCharacterSize(font_size);
			m_text.setFillColor(sf::Color(font_red, font_green, font_blue));
			m_text.setPosition(5, 0 + m_text.getLocalBounds().height);
		}
		else if (header == "Player")
		{
			fin >> m_player_config.SR >> m_player_config.CR >> m_player_config.S >> m_player_config.FR
				>> m_player_config.FG >> m_player_config.FB >> m_player_config.OR >> m_player_config.OG
				>> m_player_config.OB >> m_player_config.OT >> m_player_config.V;
		}
		else if (header == "Enemy")
		{
			fin >> m_enemy_config.SR >> m_enemy_config.CR >> m_enemy_config.SMIN >> m_enemy_config.SMAX
				>> m_enemy_config.OR >> m_enemy_config.OG >> m_enemy_config.OB >> m_enemy_config.OT
				>> m_enemy_config.VMIN >> m_enemy_config.VMAX >> m_enemy_config.L >> m_enemy_config.SI;

			m_spawn_interval= m_enemy_config.SI;
		}
		else if (header == "Bullet")
		{
			fin >> m_bullet_config.SR >> m_bullet_config.CR >> m_bullet_config.S >> m_bullet_config.FR
				>> m_bullet_config.FG >> m_bullet_config.FB >> m_bullet_config.OR >> m_bullet_config.OG
				>> m_bullet_config.OB >> m_bullet_config.OT >> m_bullet_config.V >> m_bullet_config.L;
		}
	}
		
	// set up window using config file parameters
	m_window.create(sf::VideoMode(wWidth, wHeight), "Polygon Blaster");
	m_window.setFramerateLimit(framerate);

	if (!m_background_image.loadFromFile("images/background.jpg"))
	{
		std::cerr << "Could not load background image!";
		exit(-1);
	}

	m_background.setTexture(m_background_image);
	m_background.setPosition(sf::Vector2f(0, 0));
	m_background.setScale(sf::Vector2f(wWidth / 1920.0f, wHeight / 1080.0f));

	m_tether_meter_outline= sf::RectangleShape(sf::Vector2f(180.4f, 19));
	m_tether_meter_outline.setFillColor(sf::Color(0, 0, 0, 0));
	m_tether_meter_outline.setOutlineColor(sf::Color(255, 223, 0));
	m_tether_meter_outline.setOutlineThickness(2);
	m_tether_meter_outline.setPosition(5, m_window.getSize().y - m_tether_meter_outline.getLocalBounds().height - 2);

	m_tether_meter_progress= sf::RectangleShape(sf::Vector2f(1, 19));
	m_tether_meter_progress.setFillColor(sf::Color(255, 223, 0));
	m_tether_meter_progress.setPosition(5, m_window.getSize().y - m_tether_meter_progress.getLocalBounds().height - 6);

	spawn_player();

	srand(time(0));
}

void Game::run()
{
	while (m_running)
	{
		m_entities.update();

		if (!m_paused)
		{
			s_enemy_spawner();
			s_movement();
			s_life_span();
			s_debuffs();
			s_collision();
			s_difficulty();
			m_current_frame++;
		}

		//s_collision();
		s_user_input();
		s_render();	
	}
}

void Game::set_paused()
{
	m_paused= !m_paused;
}

// respawn the player in the middle of the screen
void Game::spawn_player()
{
	// Create an entity by calling the enitity manager and give it the P tag for Player
	auto entity= m_entities.add_entity(e_Tag::P);

	// Give this entity a Transform so it spawns at the center of the window with a velocity of (0, 0) and an angle of 0
	entity->c_Transform= std::make_shared<c_Transform>(c_Vec2(m_window.getSize().x / 2, m_window.getSize().y / 2), c_Vec2(0.0f, 0.0f), 0.0f);

	// The entity's shape will have a radius, vertices, RGB fill color, RGB outline color, and outline thickness determined by the config file
	entity->c_Circle= std::make_shared<c_Circle>(m_player_config.SR, m_player_config.V,
		sf::Color(m_player_config.FR, m_player_config.FG, m_player_config.FB),
		sf::Color(m_player_config.OR, m_player_config.OG, m_player_config.OB), m_player_config.OT);

	// Add an input component to the player so that we can use inputs
	entity->c_Input= std::make_shared<c_Input>();

	// Gives the entity a collision radius determined by the config file
	entity->c_Collision= std::make_shared<c_Collision>(m_player_config.CR);

	// Since we want this Entity to be our player, set our Game's player variable to be this Entity
	// This goes slighly against the EntityManager paradigm, but we use the player so much it's worth it
	m_player= entity;
}

// spawn an enemy at a random position
void Game::spawn_enemy()
{
	// spawns an enemy entitiy with the values from the config file as parameters
	auto entity= m_entities.add_entity(e_Tag::E);

	int random_x_position= m_enemy_config.CR + (rand() % (1 + (m_window.getSize().x - m_enemy_config.CR) - m_enemy_config.CR));
	int random_y_position= m_enemy_config.CR + (rand() % (1 + (m_window.getSize().y - m_enemy_config.CR) - m_enemy_config.CR));

	int random_vertices= m_enemy_config.VMIN + (rand() % (1 + m_enemy_config.VMAX - m_enemy_config.VMIN));

	// array of sfml colors to randomly choose from for the enemy fill color
	const sf::Color color_array[6] ={ sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan };

	sf::Color color= color_array[rand() % 6];

	// start the enemy with velocity of zero 
	entity->c_Transform= std::make_shared<c_Transform>(c_Vec2(random_x_position, random_y_position), c_Vec2(0, 0), 0.0f);

	// start the enemy with an alpha value of 0 so it can be faded in
	entity->c_Circle= std::make_shared<c_Circle>(m_enemy_config.SR, random_vertices, sf::Color(color.r, color.g, color.b, 0),
		sf::Color(m_enemy_config.OR, m_enemy_config.OG, m_enemy_config.OB, 0), m_enemy_config.OT);

	// give the enemy an Ethereal component which indicates the enemy needs to be faded in and 'solidified' 
	entity->c_Ethereal= std::make_shared<c_Ethereal>(120);

	entity->c_Score= std::make_shared <c_Score>(100 * random_vertices);

	// record when the most recent enemy was spawned
	m_last_enemy_spawn_time= m_current_frame;
}

// spawns the small enemies when a big (input entity e) explodes
void Game::spawn_small_enemies(std::shared_ptr<Entity> e)
{
	//  spawn small enemies at the location of the input enemy e

	int vertices= e->c_Circle->circle.getPointCount();
	float angle= (2.0f * 3.14f) / vertices;

	for (int i= 0; i < vertices; i++)
	{
		auto small_enemy= m_entities.add_entity(e_Tag::S);

		small_enemy->c_Transform= std::make_shared<c_Transform>(c_Vec2(e->c_Transform->pos.x, e->c_Transform->pos.y), c_Vec2(cosf(angle*i), sinf(angle*i)), 0.0f);

		small_enemy->c_Circle= std::make_shared<c_Circle>(m_enemy_config.SR / 2.0f, vertices, sf::Color(e->c_Circle->circle.getFillColor()),
			sf::Color(m_enemy_config.OR, m_enemy_config.OG, m_enemy_config.OB), m_enemy_config.OT / 2.0f);

		small_enemy->c_Collision= std::make_shared<c_Collision>(m_enemy_config.CR);

		small_enemy->c_Lifespan= std::make_shared<c_Lifespan>(m_enemy_config.L);

		small_enemy->c_Score= std::make_shared<c_Score>(e->c_Score->score * 2);
	}
}

// spawns a bullet from a given entity to a target location with parameters from the config file
void Game::spawn_bullet(std::shared_ptr<Entity> entity, const c_Vec2 &target)
{
	// spawning of a bullet which travels toward target
	//		 - bullet speed is given as a scalar speed

	// get the vector distance between the player and the mouse click
	c_Vec2 vector_distance= target - entity->c_Transform->pos;

	// the angle between the player and the mouse click
	float angle= atan2f(vector_distance.y, vector_distance.x);

	float speed= m_bullet_config.S;

	auto bullet= m_entities.add_entity(e_Tag::B);

	// normalize the bullet's vector with the config file bullet speed
	bullet->c_Transform= std::make_shared<c_Transform>(c_Vec2(entity->c_Transform->pos), c_Vec2(speed*cosf(angle), speed*sinf(angle)), 0.0f);

	bullet->c_Circle= std::make_shared<c_Circle>(m_bullet_config.SR, m_bullet_config.V, sf::Color(m_bullet_config.FR, m_bullet_config.FG, m_bullet_config.FB),
		sf::Color(m_bullet_config.OR, m_bullet_config.OG, m_bullet_config.OB), m_bullet_config.OT);

	bullet->c_Collision= std::make_shared<c_Collision>(m_bullet_config.CR);

	bullet->c_Lifespan= std::make_shared<c_Lifespan>(m_bullet_config.L);
}

// spawns a special bullet, tether shot, this method functions essentially
// the same as a normal bullet spawn but a tether shot has a rectangle shape
void Game::spawn_tether_shot(std::shared_ptr<Entity> entity, const c_Vec2 &target)
{
	if (m_current_frame >= m_last_tether_shot_time + 1800)
	{
		c_Vec2 vector_distance= target - entity->c_Transform->pos;

		float angle= atan2f(vector_distance.y, vector_distance.x);

		float speed= 20.0f;

		auto tether_shot= m_entities.add_entity(e_Tag::T);

		tether_shot->c_Transform= std::make_shared<c_Transform>(c_Vec2(entity->c_Transform->pos), c_Vec2(speed * cosf(angle), speed * sinf(angle)), 0.0f);

		tether_shot->c_Rectangle= std::make_shared<c_Rectangle>(100.0f, 5.0f, angle, sf::Color(138, 43, 226));

		m_last_tether_shot_time= m_current_frame;
	}
}

// when the tether shot collides this method is called and spawns a tether field entity
void Game::spawn_tether_field(std::shared_ptr<Entity> e)
{
	auto entity= m_entities.add_entity(e_Tag::TF);

	entity->c_Transform= std::make_shared<c_Transform>(c_Vec2(e->c_Transform->pos.x, e->c_Transform->pos.y), c_Vec2(0, 0), 0.0f);

	entity->c_Circle= std::make_shared<c_Circle>(250.0f, 50, sf::Color(138, 43, 226, 75),
		sf::Color(138, 43, 226, 175), 4.0f);

	entity->c_Collision= std::make_shared<c_Collision>(250.0f);

	entity->c_Lifespan= std::make_shared<c_Lifespan>(480);
}

void Game::s_movement()
{
	// modify the player vector based in input values
	c_Vec2 playerVelocity;
	if (m_player->c_Input->left)
	{
		playerVelocity.x-= m_player_config.S;
	}
	if (m_player->c_Input->right)
	{
		playerVelocity.x+= m_player_config.S;
	}
	if (m_player->c_Input->up)
	{
		playerVelocity.y-= m_player_config.S;
	}
	if (m_player->c_Input->down)
	{
		playerVelocity.y+= m_player_config.S;
	}

	// check if the player's speed is greater than the config speed and normalizes it to the config speed
	if (playerVelocity.get_magnitude_squared() > m_player_config.S*m_player_config.S)
	{
		playerVelocity*= m_player_config.S * playerVelocity.quick_inverse_magnitude();

	}
	
	m_player->c_Transform->velocity = playerVelocity;
	
	// update all entities positions with their movement speed
	for (auto &e : m_entities.get_entities())
	{
		// the tethered affect caused by the tether field slows enemies down
		if (e->c_Tethered)
		{
			e->c_Transform->pos+= e->c_Transform->velocity / 10;
		}
		else
		{
			e->c_Transform->pos+= e->c_Transform->velocity;
		}
	}
}

void Game::s_life_span()
{
	for (auto &e : m_entities.get_entities())
	{
		if (e->c_Lifespan)
		{
			if (e->c_Lifespan->remaining > 0)
			{
				e->c_Lifespan->remaining--;

				if (e->tag() == e_Tag::S)		// if its a small enemy then reduce the alpha to fade the enemy out
				{
					// get a new alpha value reduced proportionally to the small enemy's lifespan
					int newAlpha= e->c_Circle->circle.getFillColor().a - 255 / m_enemy_config.L;

					// get the old fill color and outline color
					auto fill_color= e->c_Circle->circle.getFillColor();
					auto outline_color= e->c_Circle->circle.getOutlineColor();

					// create a new fill color with the old color and new alpha value
					sf::Color new_fill_color(fill_color.r, fill_color.g, fill_color.b, newAlpha);
					sf::Color new_outline_color(outline_color.r, outline_color.g, outline_color.b, newAlpha);

					// set the fill color and outline color with the new alpha value 
					e->c_Circle->circle.setFillColor(new_fill_color);
					e->c_Circle->circle.setOutlineColor(new_outline_color);
				}
			}
			else if (e->c_Lifespan->remaining == 0)
			{
				e->destroy();
			}
		}
	}
}

void Game::s_collision()
{
	m_spawn_player_called= false;

	// check for collisions between players and enemies
	for (auto &p : m_entities.get_entities(e_Tag::P))
	{
		for (auto &e : m_entities.get_entities(e_Tag::E))
		{
			if (e->c_Collision)
			{
				// if there's a collision between the player and an enemy
				if (p->c_Transform->pos.get_distance_squared(e->c_Transform->pos) < ((p->c_Collision->radius + e->c_Collision->radius) * (p->c_Collision->radius + e->c_Collision->radius)))
				{
					//	- destroy the player
					p->destroy();
					//	- destroy the enemy
					e->destroy();
					//  - spawn small enemies
					spawn_small_enemies(e);
					//	- respawn the player if they have not been spawned this frame already
					if (!m_spawn_player_called)
					{
						spawn_player();
						m_spawn_player_called= true;
					}
					//  - reset the score
					m_score= 0;
				}
			}
		}

		for (auto &s : m_entities.get_entities(e_Tag::S))
		{
			// if there's a collision between the player and a small enemy
			if (p->c_Transform->pos.get_distance_squared(s->c_Transform->pos) < ((p->c_Collision->radius + s->c_Collision->radius) * (p->c_Collision->radius + s->c_Collision->radius)))
			{
				//	- destroy the player
				p->destroy();
				//	- destroy the enemy
				s->destroy();
				//	- respawn the player if they have not been spawned this frame already
				if (!m_spawn_player_called)
				{
					spawn_player();
					m_spawn_player_called= true;
				}
			}
		}
	}

	// check for collisions between bullets and enemies
	for (auto &b : m_entities.get_entities(e_Tag::B))
	{
		for (auto &e : m_entities.get_entities(e_Tag::E))
		{
			if (e->c_Collision)
			{
				// if there's a collision between a bullet and an enemy
				if (b->c_Transform->pos.get_distance_squared(e->c_Transform->pos) < ((b->c_Collision->radius + e->c_Collision->radius) * (b->c_Collision->radius + e->c_Collision->radius)))
				{
					//	- destroy the bullet
					b->destroy();
					//	- destroy the enemy
					e->destroy();
					//  - spawn small enemies
					spawn_small_enemies(e);
					//  - increment the score
					m_score+= e->c_Score->score;
					if (m_score > m_high_score)
					{
						m_high_score= m_score;
					}
				}
			}
		}

		for (auto &s : m_entities.get_entities(e_Tag::S))
		{
			// if there's a collision between a bullet and a small enemy
			if (b->c_Transform->pos.get_distance_squared(s->c_Transform->pos) < ((b->c_Collision->radius + s->c_Collision->radius) * (b->c_Collision->radius + s->c_Collision->radius)))
			{
				//	- destroy the bullet
				b->destroy();
				//	- destroy the small enemy
				s->destroy();
				//  - increment the score
				m_score+= s->c_Score->score;
				if (m_score > m_high_score)
				{
					m_high_score= m_score;
				}
			}
		}
	}

	// check for collisions between tether shots and enemies
	for (auto &t : m_entities.get_entities(e_Tag::T))
	{
		for (auto &e : m_entities.get_entities(e_Tag::E))
		{
			if (e->c_Collision)
			{
				// if there's a collision between a tether shot and an enemy
				if (t->c_Rectangle->rectangle.getGlobalBounds().intersects(e->c_Circle->circle.getGlobalBounds()))
				{
					//	- destroy the tether shot
					t->destroy();
					//	- destroy the enemy
					e->destroy();
					//  - spawn small enemies
					spawn_small_enemies(e);
					//  - spawn the tether field
					spawn_tether_field(t);
					//  - increment the score
					m_score+= e->c_Score->score;
					if (m_score > m_high_score)
					{
						m_high_score= m_score;
					}
				}
			}
		}

		for (auto &s : m_entities.get_entities(e_Tag::S))
		{
			// if there's a collision between a tether shot and a small enemy
			if (t->c_Rectangle->rectangle.getGlobalBounds().intersects(s->c_Circle->circle.getGlobalBounds()))
			{
				//	- destroy the tether shot
				t->destroy();
				//	- destroy the enemy
				s->destroy();
				//  - spawn the tether field
				spawn_tether_field(t);
				//  - increment the score
				m_score+= s->c_Score->score;
				if (m_score > m_high_score)
				{
					m_high_score= m_score;
				}
			}
		}
	}

	// for all tether fields if theres a collision with an enemy, apply tethered condition to the enemy
	for (auto &t : m_entities.get_entities(e_Tag::TF))
	{
		for (auto &e : m_entities.get_entities(e_Tag::E))
		{
			if (e->c_Collision)
			{
				// if there's a collision between a tether field and an enemy
				if (t->c_Transform->pos.get_distance_squared(e->c_Transform->pos) < ((t->c_Collision->radius + e->c_Collision->radius) * (t->c_Collision->radius + e->c_Collision->radius)))
				{
					if (!e->c_Tethered)
					{
						e->c_Tethered= std::make_shared<c_Tethered>(t->c_Transform->pos, t->c_Lifespan->remaining);
					}
				}
			}
		}

		for (auto &s : m_entities.get_entities(e_Tag::S))
		{
			// if there's a collision between a tether field and a small enemy
			if (t->c_Transform->pos.get_distance_squared(s->c_Transform->pos) < ((t->c_Collision->radius + s->c_Collision->radius) * (t->c_Collision->radius + s->c_Collision->radius)))
			{
				if (!s->c_Tethered)
				{
					s->c_Tethered= std::make_shared<c_Tethered>(t->c_Transform->pos, t->c_Lifespan->remaining);
				}
			}
		}
	}

	// Bounces enemies off the window borders
	for (auto &e : m_entities.get_entities(e_Tag::E))
	{
		if (e->c_Collision)
		{
			if ((e->c_Transform->pos.x - e->c_Collision->radius <= 0) || (e->c_Transform->pos.x + e->c_Collision->radius >= m_window.getSize().x))
			{
				e->c_Transform->velocity.x*= -1.0f;
			}
			if ((e->c_Transform->pos.y - e->c_Collision->radius <= 0) || (e->c_Transform->pos.y + e->c_Collision->radius >= m_window.getSize().y))
			{
				e->c_Transform->velocity.y*= -1.0f;
			}
		}
	}

	// for every player entity, if it leaves the bounds of the window, reset it's position to within the bounds
	for (auto &p : m_entities.get_entities(e_Tag::P))
	{
		if (p->c_Transform->pos.x - p->c_Collision->radius <= 0)
		{
			p->c_Transform->pos.x= 0.0f + p->c_Collision->radius;
		}
		else if (p->c_Transform->pos.x + p->c_Collision->radius >= m_window.getSize().x)
		{
			p->c_Transform->pos.x= m_window.getSize().x - p->c_Collision->radius;
		}

		if (p->c_Transform->pos.y - p->c_Collision->radius <= 0)
		{
			p->c_Transform->pos.y= 0.0f + p->c_Collision->radius;
		}
		else if (p->c_Transform->pos.y + p->c_Collision->radius >= m_window.getSize().y)
		{
			p->c_Transform->pos.y= m_window.getSize().y - p->c_Collision->radius;
		}
	}

	// destroys tether shots upon hitting the borders of the window
	for (auto &t : m_entities.get_entities(e_Tag::T))
	{
		if (t->c_Transform->pos.x <= 0 || t->c_Transform->pos.x >= m_window.getSize().x || t->c_Transform->pos.y <= 0 || t->c_Transform->pos.y >= m_window.getSize().y)
		{
			// destroy the tether shot
			t->destroy();
			//  - spawn the tether field
			spawn_tether_field(t);
		}
	}
}

void Game::s_enemy_spawner()
{
	if (m_current_frame >= m_last_enemy_spawn_time + m_spawn_interval)
	{
		spawn_enemy();
	}
}

void Game::s_render()
{
	// clear and then draw all the entities' circle shape

	m_window.clear();
	m_window.draw(m_background);

	// set the position of the shape based on the entity's trasnform->pos
	for (auto &e : m_entities.get_entities())
	{
		if (e->c_Circle)
		{
			e->c_Circle->circle.setPosition(e->c_Transform->pos.x, e->c_Transform->pos.y);

			// set the rotation of the shape based on the entity's transform->angle
			if (e->tag() == e_Tag::TF)
			{
				e->c_Transform->angle-= 0.25f;
			}
			else
			{
				e->c_Transform->angle+= 1.0f;
			}

			e->c_Circle->circle.setRotation(e->c_Transform->angle);

			// draw the entity's sf::CircleShape
			m_window.draw(e->c_Circle->circle);
		}

		if (e->c_Tethered)
		{
			sf::Vertex line[2]=
			{ 
				sf::Vertex(sf::Vector2f(e->c_Transform->pos.x, e->c_Transform->pos.y), sf::Color(138, 43, 226, 75)),
				sf::Vertex(sf::Vector2f(e->c_Tethered->tether_point.x, e->c_Tethered->tether_point.y), sf::Color(138, 43, 226, 75))
			};

			m_window.draw(line, 2, sf::Lines);
		}
	}

	// set the position of the tether shot based on it's transform->pos
	for (auto &t : m_entities.get_entities(e_Tag::T))
	{
		t->c_Rectangle->rectangle.setPosition(t->c_Transform->pos.x, t->c_Transform->pos.y);
		m_window.draw(t->c_Rectangle->rectangle);
	}

	update_tether_meter();

	m_window.draw(m_tether_meter_outline);
	m_window.draw(m_tether_meter_progress);

	// Set the text for the score
	m_text.setString("High Score " + std::to_string(m_high_score) + "   Score " + std::to_string(m_score));
	m_window.draw(m_text);

	m_window.display();
}

void Game::s_user_input()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		// this event triggers when the window is closed
		if (event.type == sf::Event::Closed)
		{
			m_running= false;
		}

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				std::cout << "W Key Pressed\n";
				m_player->c_Input->up= true;
				break;
			case sf::Keyboard::S:
				std::cout << "S Key Pressed\n";
				m_player->c_Input->down= true;
				break;
			case sf::Keyboard::A:
				std::cout << "A Key Pressed\n";
				m_player->c_Input->left= true;
				break;
			case sf::Keyboard::D:
				std::cout << "D Key Pressed\n";
				m_player->c_Input->right= true;
				break;
			case sf::Keyboard::Space:
				std::cout << "Space Key Pressed\n";
				set_paused();
			}
		}

		// this event is triggered when a key is released
		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				std::cout << "W Key Released\n";
				m_player->c_Input->up= false;
				break;
			case sf::Keyboard::S:
				std::cout << "S Key Released\n";
				m_player->c_Input->down= false;
				break;
			case sf::Keyboard::A:
				std::cout << "A Key Released\n";
				m_player->c_Input->left= false;
				break;
			case sf::Keyboard::D:
				std::cout << "D Key Released\n";
				m_player->c_Input->right= false;
				break;
			}
		}

		// this event is triggered when a mouse button is pressed
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (!m_paused)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
					spawn_bullet(m_player, c_Vec2(event.mouseButton.x, event.mouseButton.y));
				}

				if (event.mouseButton.button == sf::Mouse::Right)
				{
					std::cout << "Reft Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
					spawn_tether_shot(m_player, c_Vec2(event.mouseButton.x, event.mouseButton.y));
				}
			}
		}
	}
}

void Game::s_debuffs()
{
	for (auto &e : m_entities.get_entities(e_Tag::E))
	{
		if (e->c_Tethered)
		{
			if (e->c_Tethered->duration > 0)
			{
				e->c_Tethered->duration--;
			}
			else if (e->c_Tethered->duration == 0)
			{
				e->c_Tethered = NULL;
			}
		}
	}

	for (auto &s : m_entities.get_entities(e_Tag::S))
	{
		if (s->c_Tethered)
		{
			if (s->c_Tethered->duration > 0)
			{
				s->c_Tethered->duration--;
			}
			else if (s->c_Tethered->duration == 0)
			{
				s->c_Tethered = NULL;
			}
		}
	}

	for (auto &e : m_entities.get_entities(e_Tag::E))
	{
		if (e->c_Ethereal)
		{
			if (e->c_Ethereal->duration > 0)
			{
				e->c_Ethereal->duration--;

				// get a new alpha value increased proportionally with an ethereal duration of 120 frames
				int newAlpha= e->c_Circle->circle.getFillColor().a + 3;

				if (newAlpha > 225)
				{
					newAlpha= 225;
				}

				// get the old fill color and outline color
				auto fill_color= e->c_Circle->circle.getFillColor();
				auto outline_color= e->c_Circle->circle.getOutlineColor();

				// create a new fill color with the old color and new alpha value
				sf::Color new_fill_color(fill_color.r, fill_color.g, fill_color.b, newAlpha);
				sf::Color new_outline_color(outline_color.r, outline_color.g, outline_color.b, newAlpha);

				// set the fill color and outline color with the new alpha value 
				e->c_Circle->circle.setFillColor(new_fill_color);
				e->c_Circle->circle.setOutlineColor(new_outline_color);
			}
			else if (e->c_Ethereal->duration == 0)
			{
				// when an enemy is no longer ethereal "solidify" it with a velocity and collision radius
				e->c_Ethereal = NULL;

				int random_x_speed= m_enemy_config.SMIN + (rand() % (int)(1 + m_enemy_config.SMAX - m_enemy_config.SMIN));
				int random_y_speed= m_enemy_config.SMIN + (rand() % (int)(1 + m_enemy_config.SMAX - m_enemy_config.SMIN));

				e->c_Transform->velocity= c_Vec2(random_x_speed, random_y_speed);

				e->c_Collision= std::make_shared<c_Collision>(m_player_config.CR);
			}
		}
	}
}

void Game::s_difficulty()
{
	// reduces the spawn interval by one frame every 106 frames
	if (m_current_frame == m_last_difficulty_increase + 106)
	{
		// minimum enemy spawn interval of 1
		if (m_spawn_interval > 1)
		{
			m_spawn_interval--;

			m_last_difficulty_increase= m_current_frame;
		}
	}

	// if the player died this frame then reset the difficulty
	if (m_spawn_player_called)
	{
		m_spawn_interval= m_enemy_config.SI;
	}
}

// increases the tether cooldown bar with the cooldown time 
void Game::update_tether_meter()
{
	if (m_current_frame <= m_last_tether_shot_time + 1800)
	{
		m_tether_meter_progress.setSize(sf::Vector2f((m_current_frame - m_last_tether_shot_time) / 10, 19));
	}
}