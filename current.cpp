/*
 Madeeha Anjum & Pranavkumar Bodawala
*/
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <math.h>
#include <TouchScreen.h>
#include "common.h"

#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// touch screen pins, obtained from the documentaion
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM  5  // can be a digital pin
#define XP  4  // can be a digital pin

// width/height of the display when rotated horizontally
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

//touch
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// thresholds to determine if there was a touch
#define MINPRESSURE   1
#define MAXPRESSURE 1000000

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20

// the pushbutton is attached to this digital input pin
#define pushbuttonPin 8

// slider of potentiometer attached to this analog input
#define analogInPin  A7

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240
#define JOY_VERT  A1
#define JOY_HORIZ A0
#define JOY_SEL   2
#define JOY_CENTER   512
#define JOY_DEADZONE 64

#define MIDDLE_X (DISPLAY_WIDTH/2)
#define MIDDLE_Y (DISPLAY_HEIGHT/2)

#define DODGER_SPEED 100 // Lower = Higher speed
#define DODGER_SIZE 5

#define BULLET_SIZE 3
#define BULLET_SPEED 3
#define TOTAL_BULLETS 20 // Make sure this number is big enough
#define BULLET_MAX_LIFE 10
#define BULLET_LIFE_REFRESH 500 // the bullet's life will go down every "BULLET_LIFE_REFRESH" seconds
unsigned long bullet_life_time = 0;
// NOTE: how long the bullet will last in the game = BULLET_MAX_LIFE * BULLET_LIFE_REFRESH

#define COLLISION_REFRESH 50 // After the dodger is hit, it can't be damaged for this amount of time

#define DEBOUNCE_DELAY 400 // Minimum amount of time between each shot

// a multimeter reading says there are 300 ohms of resistance across the plate,
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int colorarray[255];
int temporary=0;

unsigned long lastDebounceTime = 0;  // the last time the pushbutton was toggled for the bullet

void setup() {
	init();
	Serial.begin(9600);

	// Setting up the joystick
	pinMode(JOY_SEL, INPUT_PULLUP);

	// Setting up the pushbutton
	pinMode(pushbuttonPin, INPUT);
	digitalWrite(pushbuttonPin, HIGH);

	// Setting up the display
	tft.begin();
	tft.setRotation(3);
	tft.setTextWrap(false);
	tft.fillScreen(ILI9341_BLACK);
}

//***********************************start************************************
void Startgame(){

  tft.setTextColor(ILI9341_RED);
  tft.setCursor(320/2-100,270-250); // middle
  tft.setTextSize(1);
  tft.println("PUSH BUTTON TO START GAME");

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(320/2-150,270-50);
  tft.println("CREDITS");
  tft.setCursor(320/2-100,270-50);
  tft.println("MADEEHA and ");
  tft.setCursor(320/2-30,270-50);
  tft.println("PRANAV");
  // middle
  //draw a loading screen in a while loop that ony ends after a
  // certine amoutn of time but u can exit if you push the pushbutton

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10,50); // middle
  tft.setTextSize(6);
  tft.println("LOADING");
  int changex=5;
  int width=10;
  int height=10;
  int delta=20;

  while (1){

    for (int i=0;i< 25;i++){
      tft.drawRect(changex,240/2, width,height,ILI9341_WHITE );
      tft.fillRect(changex,240/2, width,height,ILI9341_WHITE );
      changex= changex+width;
      delta=delta+30;
      delay(30+delta);
      if (digitalRead(pushbuttonPin) == LOW){
        break;
      }
    }
    break;
  }
	tft.fillScreen(ILI9341_BLACK);

  for(int i=0;i<250;i++){
    tft.setCursor(10,50);
    tft.setTextSize(2);
    tft.setTextColor(colorarray[i]);
    tft.println("WELCOME TO EVATION");
  }
  tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(WHITE);
	tft.setTextSize(1);
}

//**************************************** LIFE and SCORE ***********************/
void draw_life(int life){
	tft.setTextSize(1);
tft.setTextColor(WHITE);
  tft.fillRect(-3, -3, 44, 14, BLACK); //so the new life dosnt print over
	tft.drawRect(-2, -2, 45, 15, BLUE);
	tft.setCursor(3,3); // top left of shotter
	tft.print("LIFE ");
	tft.println(" ");

	tft.setCursor(3,3); // top left of shotter
	tft.print("LIFE ");
	tft.println(life);
}

void draw_score(int score){
	tft.setTextSize(1);
	tft.setTextColor(WHITE);
	tft.fillRect(DISPLAY_WIDTH-79,-3,79, 14,BLACK); //so the new dosnt draw over the old
	tft.drawRect(DISPLAY_WIDTH-80,-2,80, 15,BLUE);
	tft.setCursor(DISPLAY_WIDTH-70,0); // top left of shotter
	tft.print("SCORE ");
	tft.println(score); //baced on time
}

void checkTouch(Shooter &shooter) {
	TSPoint touch = ts.getPoint();

	if (touch.z < MINPRESSURE || touch.z > MAXPRESSURE) {
		// no touch, just quit
		return;
	}

	int touchY = map(touch.x, TS_MINX, TS_MAXX, 0, TFT_HEIGHT - 1);

	int touchX = map(touch.y, TS_MINY, TS_MAXY, TFT_WIDTH - 1, 0);


		if(touchX >=shooter.x - shooter.radius && touchX <= shooter.x + shooter.radius
			&& touchY >= shooter.y-shooter.radius   && touchY <= shooter.y+shooter.radius ){
				//draw the shotter
				tft.fillCircle(shooter.x, shooter.y, // middle x of the circle, middle y of the circle
					shooter.radius, RED // radius, colour
				);

			tft.setCursor(shooter.x - shooter.radius/2-10, shooter.y );
		  tft.println(" PUSH TO");
			tft.setCursor(shooter.x - shooter.radius/2-10, shooter.y + 10);
		  tft.println(" UNPAUSE");
			 PAUSE(shooter);
		}


}

//declectation of void functions
//if you push the shotting button the game strats or
//if u push the play button the game will once agin start
void PAUSE(Shooter &shooter){
	while (1){
	if (digitalRead(pushbuttonPin) == LOW){
		break;
	}
	}
	drawPlay(shooter);
}

void drawPlay(Shooter &shooter){
	// draw the shooter
	tft.fillCircle(shooter.x, shooter.y, // middle x of the circle, middle y of the circle
		shooter.radius, BLUE // radius, colour
	);
	tft.setTextSize(1);
	tft.setCursor(shooter.x - shooter.radius/2-5, shooter.y );
  tft.println(" PUSH");
	tft.setCursor(shooter.x - shooter.radius/2-5, shooter.y + 10);
  tft.println(" TO ");
	tft.setCursor(shooter.x - shooter.radius/2-5, shooter.y + 20);
  tft.println(" PAUSE");
}


//************************** Shooter **********************************************

void poteto_meter(Shooter &shooter){

	// copy the shot position (to check later if it changed)
	int oldpositionY = shooter.aim_y; // technically not needed since potentiometer only affects the x position, but helps with understanding
	int oldpositionX = shooter.aim_x;

	// map( value, fromLow, fromHigh, toLow, toHigh)
	shooter.aim_x = map(analogRead(analogInPin), 0, 1023,
	shooter.x - shooter.aim_radius + 1, shooter.x + shooter.aim_radius - 1 );
	// NOTE: added/subtracted 1 so that the bullets can't go perfectly horizontal and be stuck

	// redraw the shot only if its position actually changed
	// Since the potentiometer only affects the y position, we only need to check that
	if (shooter.aim_x != oldpositionX) {

		// if the y position changed, calculate the new x position
		shooter.aim_y = sqrt(pow(shooter.aim_radius,2)-pow(shooter.aim_x - shooter.x ,2));

		// draw a black square over the shoot old poition
		tft.fillRect( oldpositionX - shooter.aim_size/2, oldpositionY - shooter.aim_size/2,
			shooter.aim_size, shooter.aim_size, BLACK
		);

		// and now draw the aimer at the new position
		tft.fillRect( shooter.aim_x - shooter.aim_size/2, shooter.aim_y - shooter.aim_size/2,
			shooter.aim_size, shooter.aim_size, RED
		);
	}
}

// Shooting a bullet based on the current position of the aimer
void shoot_bullet(Bullet bullets[], Shooter &shooter){

	// find the first bullet in the array that is dead ("life == 0")
	for (int i = 0; i < TOTAL_BULLETS; ++i) {
		if (bullets[i].life == 0) {

			// Bring the bullet back to life. it's x and y position is the aiming position
			bullets[i].life = BULLET_MAX_LIFE;
			bullets[i].x = shooter.aim_x;
			bullets[i].y = shooter.aim_y;

			// temporary value
			double tempx = shooter.aim_x - shooter.x;

			// opp = shooter.aim_y, adj = tempx, hyp = bullet speed
			// These are equavalent to doing trig functions, but we already have all the sides so we don't need to calculate the angle
			// equavalent to cos(theta) * hyp
			bullets[i].dx = (tempx * BULLET_SPEED) / (double)shooter.aim_radius ;
			// equavilent to sin(theta) * hyp
			bullets[i].dy = (shooter.aim_y * BULLET_SPEED ) / (double)shooter.aim_radius ;

			// We have brought one bullet back to life, and that's all we need
			break;
		}
	}
}

// Controls aim via the potentiometer and shoots a bullet if button is pressed
void shooter_func(Bullet bullets[], Shooter &shooter) {

	// aim the shooter
	poteto_meter(shooter);

	// if the "DEBOUNCE_DELAY" time has passed since we last fired a bullet, check if the button is pushed to shoot another bullet
	// "millis() - lastDebounceTime" is how long it's been since we last fired a bullet
	// this prevents multiple bullets from being fired at once
	if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
		if (digitalRead(pushbuttonPin) == LOW) {
			shoot_bullet(bullets, shooter);

			// Now that we fired a bullet, updated the last debounce time
			lastDebounceTime = millis();
		}
	}
}

/************************************ Bullets *******************************/

// NOTE: Pass by reference so it doesn't copy the entire struct every time - it is more efficient
void bounceOffShooter(Bullet &curr_bullet, Shooter &shooter) {
	// Check if the bullet has hit the shooter
	// the first condition: The bullet is fired from the circle, but it bounces off a box that is bigger than the circle
	// So the bullet would bounce off the "walls" of the box just as it spawned. To fix this, the bullet can only bounce off the box
	// if it's health is less max_life to give it time to escape the box
	if ( (curr_bullet.life < BULLET_MAX_LIFE) &&
	(curr_bullet.x + curr_bullet.size_x/2 + curr_bullet.dx >= shooter.x - shooter.aim_radius) &&
	(curr_bullet.x - curr_bullet.size_x/2 + curr_bullet.dx <= shooter.x + shooter.aim_radius) &&
	(curr_bullet.y + curr_bullet.size_y/2 + curr_bullet.dy >= shooter.y - shooter.aim_radius) &&
	(curr_bullet.y - curr_bullet.size_y/2 + curr_bullet.dy <= shooter.y + shooter.aim_radius) ) {

		// We know for sure that the bullet has hit the shooter
		// So if it's y position is below the shooter, just switch it's y velocity
		if (curr_bullet.y > shooter.y + shooter.aim_radius) {curr_bullet.dy = -curr_bullet.dy;}
		// otherwise, it is on the side of the shooter, so just switch it's x velocity
		else {curr_bullet.dx = -curr_bullet.dx;}

	}
}

void bounceOffBlock(Bullet &curr_bullet, Block &block) {
	if (
		(curr_bullet.x + curr_bullet.size_x/2 + curr_bullet.dx >= block.x - block.size_x/2) &&
		(curr_bullet.x - curr_bullet.size_x/2 + curr_bullet.dx <= block.x + block.size_x/2) &&
		(curr_bullet.y + curr_bullet.size_y/2 + curr_bullet.dy >= block.y - block.size_y/2) &&
		(curr_bullet.y - curr_bullet.size_y/2 + curr_bullet.dy <= block.y + block.size_y/2) )
		{

			if (curr_bullet.y - curr_bullet.dy > block.y + block.size_y/2) {curr_bullet.dy = -curr_bullet.dy;}
			if (curr_bullet.y - curr_bullet.dy < block.y - block.size_y/2) {curr_bullet.dy = -curr_bullet.dy;}
			else {curr_bullet.dx = -curr_bullet.dx;}
		}
}

// NOTE: Pass by reference so it doesn't copy the entire struct every time - it is more efficient
void redraw_bullet(Bullet &curr_bullet, Shooter &shooter, Block &block) {

	// Draw black over the old bullet position
	tft.fillRect(curr_bullet.x - curr_bullet.size_x/2, curr_bullet.y - curr_bullet.size_y/2,
		curr_bullet.size_x, curr_bullet.size_y, ILI9341_BLACK
	);

	// Calculate new bullet position
	curr_bullet.x = curr_bullet.x + curr_bullet.dx;
	curr_bullet.y = curr_bullet.y + curr_bullet.dy;

	// If it hits the walls, make it bounce
	if (curr_bullet.y + curr_bullet.size_y/2 >= DISPLAY_HEIGHT) {curr_bullet.dy = -curr_bullet.dy;}
	if (curr_bullet.y - curr_bullet.size_y/2 <= 0) {curr_bullet.dy = -curr_bullet.dy;}
	if (curr_bullet.x + curr_bullet.size_x/2 >= DISPLAY_WIDTH) {curr_bullet.dx = -curr_bullet.dx;}
	if (curr_bullet.x - curr_bullet.size_x/2 <= 0) {curr_bullet.dx = -curr_bullet.dx;}

	// if it hits the shooter, make it bounce
	bounceOffShooter(curr_bullet, shooter);

	// if it hits the block, make it bounce
	bounceOffBlock(curr_bullet, block);

	// Draw the new bullet position
	tft.fillRect(curr_bullet.x - curr_bullet.size_x/2, curr_bullet.y - curr_bullet.size_y/2,
		curr_bullet.size_x, curr_bullet.size_y, curr_bullet.colour
	);

}

// Just a cute function to draw black over the current bullet
	void delete_bullet(Bullet &curr_bullet) {
		// Draw black over the old bullet position
		tft.fillRect(curr_bullet.x - curr_bullet.size_x/2, curr_bullet.y - curr_bullet.size_y/2,
			curr_bullet.size_x, curr_bullet.size_y, ILI9341_BLACK
		);
	}


// Redraws all the bullets that are alive
// and Decreases the life of any bullet alive every "BULLET_LIFE_REFRESH" seconds
void redraw_all_bullets(Bullet bullets[], Shooter &shooter , Block &block){
	// For each bullet
	for (int i = 0; i < TOTAL_BULLETS; ++i) {
		// If it is alive, update it's position
		if (bullets[i].life > 0 ) {
			redraw_bullet(bullets[i], shooter, block );
		}

		// After every "BULLET_LIFE_REFRESH" milli seconds, if the bullet is alive, decrease its life by one
		// if a bullet just died, destroy it by drawing black over it
		if (millis() - bullet_life_time > BULLET_LIFE_REFRESH) {

			if (bullets[i].life > 0) {
				--bullets[i].life;
				if (bullets[i].life == 0) {delete_bullet(bullets[i]);}
			}

			// reset the timer after you have gone through each of the bullets
			if (i == TOTAL_BULLETS - 1) {
				bullet_life_time = millis();

			}
		}

	}
}

/********************************* Dodger ************************************/

// Just a cute function to draw the dodger at it's current position
void draw_dodger(Dodger &dodger) {
	tft.fillRect(dodger.x - dodger.size_x/2, dodger.y - dodger.size_y/2,
		dodger.size_x, dodger.size_y, dodger.colour
	);
}

void move_dodger(int xVal, int yVal, Dodger &dodger, Shooter &shooter, Block &block) {

	// draw a black square over the old doger
	tft.fillRect(dodger.x - dodger.size_x/2, dodger.y - dodger.size_y/2,
		dodger.size_x, dodger.size_y, ILI9341_BLACK
	);

	// calculate new x and y positions
	dodger.x -= (xVal - JOY_CENTER) / DODGER_SPEED;
	dodger.y += (yVal - JOY_CENTER) / DODGER_SPEED;

	// Make sure it doesn't leave the edge of the screen
	dodger.x = constrain(dodger.x, 0 + dodger.size_x/2, DISPLAY_WIDTH - dodger.size_x/2);
	dodger.y = constrain(dodger.y, shooter.aim_radius + dodger.size_y/2, DISPLAY_HEIGHT - dodger.size_y/2);

	// draw the new dodger
	draw_dodger(dodger);

	if (abs(dodger.x - block.x) <= ((dodger.size_x + block.size_x)/2)){
		// redraw the block
		tft.fillRect(block.x - block.size_x/2, block.y - block.size_y/2, block.size_x, block.size_y, block.colour);
	}
}

// NOTE: Pass by reference so it doesn't copy the entire struct every time - it is more efficient
bool chk_collision(Bullet &curr_bullet, Dodger &dodger) {

	// If the x borders of the dodger and the bullet collide
	bool x_collision = abs(curr_bullet.x - dodger.x) <= ((BULLET_SIZE + DODGER_SIZE)/2);
	// And the y borders also collide...
	bool y_collision = abs(curr_bullet.y - dodger.y) <= ((BULLET_SIZE + DODGER_SIZE)/2);

	// They have collided
	if (x_collision && y_collision) { return true;}
	// otherwise, they have not
	else {return false;}
}

//*********************************end game ************************************

void coustumcolorfade(){
    for(int i=0;i<254;i++){
      //want to go to black 255,255,255
      uint16_t color =tft.color565(255-i,255-i,255-i);
      colorarray[i]= color;
    }
}

void EndGame(int currentscore){

	int score=currentscore;

	for(int i=0;i<254;i++){
    tft.setCursor(10,200); // middle
    tft.setTextSize(5);
    tft.setTextColor(colorarray[i]);
    tft.println("YOU DIED!!");
  }
	tft.setCursor(10,150);
  tft.setTextColor( ILI9341_RED);
  tft.setTextSize(2.5);
  tft.println( "GAME OVER");
	tft.print ("current score  ");
	tft.println(score);
	tft.println("Push to play again!");
 		//draw_score() draw the score
		while(1){
			if (digitalRead(pushbuttonPin) == LOW){
				tft.fillRect(0,40, DISPLAY_WIDTH, DISPLAY_HEIGHT,BLACK);
					for(int i=0;i<254;i++){
						tft.setCursor(10,200); // middle
						tft.setTextSize(5);
						tft.setTextColor(colorarray[i]);
						tft.println("EVATION!!");
					}
				break;
			}
		}

}

/******************************************************** MAIN *************************************************************/
int main() {

	setup();
	coustumcolorfade();
	Startgame();

	// Create all bullets. NOTE: it is an array of structs
	Bullet bullets[TOTAL_BULLETS];
	for (int i = 0; i < TOTAL_BULLETS; ++i) {
		bullets[i].x = 0;
		bullets[i].y = 0;
		bullets[i].dx = 0;
		bullets[i].dy = 0;
		bullets[i].size_x = BULLET_SIZE;
		bullets[i].size_y = BULLET_SIZE;
		bullets[i].life = 0;
		bullets[i].colour = WHITE;
	}


	// Create Dodger
	Dodger dodger;
	dodger.x = MIDDLE_X;
	dodger.y = MIDDLE_Y;
	dodger.size_x = DODGER_SIZE;
	dodger.size_y = DODGER_SIZE;
	dodger.life = 3;
	dodger.colour = YELLOW;

	bool collision = false; // it hasn't collided with a bullet yet

	// Draw the dodger
	draw_dodger(dodger);

	// Initially variables that hold information on the joystick's x and y vals
	int xVal;
	int yVal;


	// Create Shooter
	// NOTE: the aiming and shooting rely on the fact that it is on the top of the screen
	Shooter shooter;
	shooter.x = 160;
	shooter.y = 0;
	shooter.radius = 30;
	shooter.aim_radius = 40;
	shooter.aim_x = MIDDLE_X;
	shooter.aim_y = shooter.aim_radius;
	shooter.aim_size = 2;

	// draw the shooter
	tft.fillCircle(shooter.x, shooter.y, // middle x of the circle, middle y of the circle
		shooter.radius, BLUE // radius, colour
	);

	// draw a circle around the shooter (for asthetics)
	tft.drawCircle(shooter.x, shooter.y, // middle x of the circle, middle y of the circle
		shooter.radius + 5, YELLOW // radius, colour
	);

	// What the shooter "box" looks like. This is what the bullets are actually bouncing off of
	// tft.fillRect(shooter.x - shooter.aim_radius, shooter.y - shooter.aim_radius,
	// 	shooter.aim_radius*2,shooter.aim_radius*2, ILI9341_WHITE
	// );

	// What the time was at the last collision
	unsigned long collision_time = 0;

	// Draw current life
	draw_life(dodger.life);

	// Draw current score
	int score = 1;
	unsigned long score_time = 0; // What the time was at the last time the score was changed
	draw_score(score);

	drawPlay(shooter);

	Block block;
	block.x = MIDDLE_X;
	block.y = MIDDLE_Y;
	block.size_x = 80;
	block.size_y = 2;
	block.colour = GREEN;

	tft.fillRect(block.x - block.size_x/2, block.y - block.size_y/2,
		block.size_x, block.size_y, block.colour
	);


	// MAIN GAME LOOP
	while (true) {

		// Refresh rate of the game
		delay(10);

		// If one second has passed since the last time we updated the score, increase the score by one
		if (millis() - score_time > 1000) {score += 1; draw_score(score); score_time = millis(); draw_life(dodger.life);}

		// Check if the shooter has been touch to pause the game
		checkTouch(shooter);

		shooter_func(bullets, shooter); // loading the shooter with bullets

		// Update and redraw all the bullets
		redraw_all_bullets(bullets, shooter, block);


		// The outermost if statement is to make sure that the dodger can't get hit twice because a bullet went throught it
		// After the dodger is hit, the program will wait until "COLLISION_REFRESH" time has passed to check for collisions again
		if (millis() - collision_time > COLLISION_REFRESH) {

			// For each bullet, check if the dodger has collided with it
			for (int i = 0; i < TOTAL_BULLETS; ++i){
				collision = chk_collision(bullets[i], dodger);

				// if the bullet we are currenly look at has collided with the dodger, decrease the dodger's life
				if (collision) {
					--dodger.life;

					// Change the dodger's colour depending on how many lives it has
					if (dodger.life == 2) {dodger.colour = ORANGE;}
					if (dodger.life == 1) {dodger.colour = RED;}
					if (dodger.life == 0) {
					EndGame(score);

					for (int i = 0; i < TOTAL_BULLETS; ++i) {
						bullets[i].x = 0;
						bullets[i].y = 0;
						bullets[i].dx = 0;
						bullets[i].dy = 0;
						bullets[i].size_x = BULLET_SIZE;
						bullets[i].size_y = BULLET_SIZE;
						bullets[i].life = 0;
						bullets[i].colour = WHITE;
					}

					redraw_all_bullets(bullets, shooter, block);
					dodger.colour=YELLOW;
					dodger.life=3;
					score=1;
					score_time = 0;
					//draw block
					tft.fillRect(block.x - block.size_x/2, block.y - block.size_y/2,
						block.size_x, block.size_y, block.colour
					);

				}


					// update the dodger's life
					draw_life(dodger.life);

					// draw the dodger with it's new colour
					draw_dodger(dodger);

					// reset the timer. Now the program must wait until "COLLISION_REFRESH" time has passed to check for collisions again
					collision_time = millis();
				}

				// now that we have dealt with the collision, change it to false
				collision = false;
			}

		}




		// Read x and y values of the dodger. If the joystick moved, update the dodger
		xVal = analogRead(JOY_HORIZ);
		yVal = analogRead(JOY_VERT);
		if ((xVal > (JOY_CENTER + JOY_DEADZONE)) || (xVal < (JOY_CENTER - JOY_DEADZONE)) ||
		(yVal > (JOY_CENTER + JOY_DEADZONE)) || (yVal < (JOY_CENTER - JOY_DEADZONE)))
		{
			move_dodger(xVal, yVal, dodger, shooter, block);
		}



	}

	Serial.end();

	return 0;
}
