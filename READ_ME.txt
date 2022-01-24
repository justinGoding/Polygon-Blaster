Controls:
Movement	:	WASD
Normal fire	:	Left Mouse
Special fire	:	Right Mouse
Pause		:	Space Bar

Config format:

Window W H FL FS
  Width			W		int
  Height		H		int
  Frame Limit		FL		int
  Full-screen mode	FS		int (1 or 0)

Font F S R G B
  Font file		F		std::string (no spaces)
  Font Size		S		int
  RGB Color		R, G, B		int, int, int

Player Specification:
Player SR CR S FR FG FB OR OG OB OT V
  Shape Radius		SR		int
  Collision Radius	CR		int
  Speed			S		float
  Fill Color		FR, FG, FB	int, int, int
  Outline Color		OR, OG, OB	int, int, int
  Outline Thickness	OT		int
  Shape Vertices	V		int

Enemy Specifications:
Enemy SR CR SMIN SMAX OR OG OB OT VMIN VMAX L SI
  Shape Radius		SR		int
  Collision Radius	CR		int
  Min / Max Speed	SMIN, SMAX	float, float
  Outline Color		OR, OG, OB	int, int, int
  Outline Thickness	OT		int
  Min / Max Vertices	VMIN, VMAX	int, int
  Small Lifespan	L		int
  Spawn Interval	SI		int

Bullet Specification:
Bullet SR CR S FR FG FB OR OG OB OT V L
  Shape Radius		SR		int
  Collision Radius	CR		int
  Speed			S		float
  Fill Color		FR, FG, FB	int, int, int
  Outline Color		OR, OG, OB	int, int, int
  Outline Thickness	OT		int
  Shape Vertices	V		int
  Lifespan		L		int