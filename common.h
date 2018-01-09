#ifndef COMMON_H
#define COMMON_H

extern Adafruit_ILI9341 tft;


struct Bullet {
	double x; // refers to the middle of the bullet
	double y; // refers to the middle of the bullet
	double dx; // x velocity of the bullet
	double dy; // y velocity of the bullet
	int size_x;
	int size_y;
	int life; // Total life of the bullet
	uint16_t colour;
};

struct Dodger {
	int x;
	int y;
	int size_x;
	int size_y;
	int life;
	uint16_t colour;
};

struct Shooter {

	int x; // middle of the shooter
	int y; // middle of the shooter
	int radius; // Radius of the shooter
	double aim_x; // x coordinate of the middle of the "aimer"
	double aim_y; // y coordinate of the middle of the "aimer"
	int aim_radius; // radius of the "aimer"
	int aim_size; // how big the square of the aimer is
};
struct Block {
	int x;
	int y;
	int size_x;
	int size_y;
	uint16_t colour;
};


void setup();

void coustumcolorfade();
void Startgame();
void EndGame(int currentscore);
void draw_life(int life);
void draw_score(int score);
void PAUSE(Shooter &shooter);
void checkTouch(Shooter &shooter);
void drawPlay(Shooter &shooter);
void poteto_meter(Shooter &shooter);
void shoot_bullet(Bullet bullets[], Shooter &shooter);
void shooter_func(Bullet bullets[], Shooter &shooter);
void redraw_bullet(Bullet &curr_bullet, Shooter &shooter, Block &block);
void delete_bullet(Bullet &curr_bullet);
void redraw_all_bullets(Bullet bullets[], Shooter &shooter, Block &block);
void draw_dodger(Dodger &dodger);
void bounceOffBlock(Bullet &curr_bullet, Block &block);
void bounceOffShooter(Bullet &curr_bullet, Shooter &shooter);
void move_dodger(int xVal, int yVal, Dodger &dodger, Shooter &shooter);
bool chk_collision(Bullet &curr_bullet, Dodger &dodger);



#endif
