#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <list>
using namespace sf;

float degree_to_radian = 0.017453f;

const int W = 1500;
const int H = 1000;

class Animation
{
public:
	float Frame, speed;
	Sprite sprite;
    std::vector<IntRect> frames;

	Animation(){}

    Animation (Texture &t, int x, int y, int w, int h, int count, float Speed)
	{
	    Frame = 0;
        speed = Speed;

		for (int i=0;i<count;i++)
         frames.push_back( IntRect(x+i*w, y, w, h)  );

		sprite.setTexture(t);
		sprite.setOrigin(w/2,h/2);
        sprite.setTextureRect(frames[0]);
	}


	void update()
	{
    	Frame += speed;
		int n = frames.size();
		if (Frame >= n) Frame -= n;
		if (n>0) sprite.setTextureRect( frames[int(Frame)] );
	}

	bool isEnd()
	{
	  return Frame+speed>=frames.size();
	}

};


class Entity
{
public:
float x,y,dx,dy,R,angle;
bool life;
std::string name;
Animation anim;

Entity()
{
  life=1;
}

void settings(Animation &a,int X,int Y,float Angle=0,int radius=1)
{
  anim = a;
  x=X; y=Y;
  angle = Angle;
  R = radius;
}

virtual void update(){};

void draw(RenderWindow &app)
{
  anim.sprite.setPosition(x,y);
  anim.sprite.setRotation(angle+90);
  app.draw(anim.sprite);

  CircleShape border(R);
  border.setFillColor(Color(255,0,0,170));
  border.setPosition(x,y);
  border.setOrigin(R,R);
  //app.draw(border);
}

virtual ~Entity(){};
};


class asteroid: public Entity
{
public:
  asteroid()
  {
    dx=rand()%8-4;
    dy=rand()%8-4;
	if(!dx && !dy){
		dy = dx = 1;
	}
    name="asteroid";
  }

void  update()
  {
   x+=dx;
   y+=dy;

   if (x>W) x=0;  if (x<0) x=W;
   if (y>H) y=0;  if (y<0) y=H;
  }

};



class bullet: public Entity
{
public:
  bullet()
  {
    name="bullet";
  }

void  update()
  {
   dx=cos(angle*degree_to_radian)*13;
   dy=sin(angle*degree_to_radian)*13;
  // angle+=rand()%6-3;
   x+=dx;
   y+=dy;

   if (x>W || x<0 || y>H || y<0) life=0;
  }

};


class space_ship: public Entity
{
public:
   bool thrust;
   bool brake;
   int score;
   int lives_left;

   space_ship()
   {
     name="player";
	 score = 0;
	 lives_left = 5;
   }

   void update()
   {
     if (thrust)
      { dx+=cos(angle*degree_to_radian)*0.2;
        dy+=sin(angle*degree_to_radian)*0.2;}
	 else if(brake){
		dx*=0.93;
		dy*=0.93;
		//for reverse thruster
		//dx-=cos(angle*degree_to_radian)*0.2;
        //dy-=sin(angle*degree_to_radian)*0.2;
	 }
     else
      { dx*=0.99;
        dy*=0.99; }

    int maxSpeed=11;
    float speed = sqrt(dx*dx+dy*dy);
    if (speed>maxSpeed)
     { dx *= maxSpeed/speed;
       dy *= maxSpeed/speed; }

    x+=dx;
    y+=dy;

    if (x>W) x=0; if (x<0) x=W;
    if (y>H) y=0; if (y<0) y=H;
   }

};


bool isCollide(Entity *a,Entity *b)
{
  return (b->x - a->x)*(b->x - a->x)+
         (b->y - a->y)*(b->y - a->y)<
         (a->R + b->R)*(a->R + b->R);
}


int main()
{
    srand(time(NULL));

    RenderWindow app(VideoMode(W, H), "Space Shooter");
    app.setFramerateLimit(60);

	const char* backgrounds[] = {
		"images/background.jpg",
		"images/background1.jpg",
		"images/background2.jpg",
		"images/background3.jpg"
	};

	int bgd = rand()%4;

    Texture t1,t2,t3,t4,t5,t6,t7;
    t1.loadFromFile("images/spaceship.png");
    t2.loadFromFile(backgrounds[bgd]);
    t3.loadFromFile("images/explosions/type_C.png");
    t4.loadFromFile("images/rock.png");
    t5.loadFromFile("images/fire_red.png");
    t6.loadFromFile("images/rock_small.png");
    t7.loadFromFile("images/explosions/type_B.png");
	//t8.loadFromFile("images/scoreboard.jpg");
	//t9.loadFromFile("images/scoreboard2.jpg");

    t1.setSmooth(true);
    t2.setSmooth(true);
	//t8.setSmooth(true);
	//t9.setSmooth(true);

    Sprite background(t2);
	//Sprite scoreboard(t8);
	//Sprite lifeboard(t9);

	std::string player_score_file,player_score_game;

	std::ifstream high_score_read;
	high_score_read.open("high_score/high_score.txt");

	Sprite life_sprites[5];

	for(int i = 0; i < 5; i++){
		life_sprites[i].setTexture(t1);
		life_sprites[i].setTextureRect(IntRect(40,0,40,40));
		life_sprites[i].setOrigin(20,20);
	}

	for(int i = 0; i < 5; i++){
		life_sprites[i].setPosition(40+i*40, 6*H/7+100);
	}
	/*
	life_sprites[2].setPosition(130,6*H/7 + 100);
	life_sprites[1].setPosition(90,6*H/7 + 100);
	life_sprites[0].setPosition(50,6*H/7 + 100);*/

	//scoreboard.setPosition(4*W/9,0);
	//lifeboard.setPosition(0,5*H/6);

	Text display_score;
	Font score_font;
	score_font.loadFromFile("fonts/space_font.TTF");
	display_score.setFont(score_font);

	display_score.setCharacterSize(24);
	display_score.setFillColor(Color::White);
	//display_score.setStyle(Text::Bold);
	display_score.setPosition(W/2 - 25, 10);

	Text end_text;
	Font end_font;
	end_font.loadFromFile("fonts/game_over.TTF");
	end_text.setFont(end_font);

	end_text.setString("GAME OVER");
	end_text.setCharacterSize(30);
	end_text.setFillColor(Color::White);
	//end_text.setStyle(Text::Bold);
	end_text.setPosition(W/2 - 250,H/2 - 40);

	Text new_high_score;
	new_high_score.setFont(end_font);
	new_high_score.setString("NEW HIGH SCORE");
	new_high_score.setCharacterSize(20);
	new_high_score.setFillColor(Color::White);
	new_high_score.setPosition(W/2 - 240,H/2 + 40);

	Text ships_left;
	ships_left.setFont(score_font);

	ships_left.setString("SHIPS LEFT");
	ships_left.setCharacterSize(16);
	ships_left.setFillColor(Color::White);
	ships_left.setPosition(20,6*H/7 + 50);

	Text high_score_screen;
	high_score_screen.setFont(score_font);

	high_score_screen.setString("HIGH SCORE");
	high_score_screen.setCharacterSize(16);
	high_score_screen.setFillColor(Color::White);
	high_score_screen.setPosition( W - 213, 6*H/7 + 50);

	Text high_score_val;
	high_score_val.setFont(score_font);

	getline(high_score_read,player_score_file);
	high_score_read.close();

	int p_score_file_val = std::atoi(player_score_file.c_str());

	high_score_val.setString(player_score_file);
	high_score_val.setCharacterSize(16);
	high_score_val.setFillColor(Color::White);
	high_score_val.setPosition( W - 117, 6*H/7 + 85);

    Animation sExplosion(t3, 0,0,256,256, 48, 0.5);
    Animation sRock(t4, 0,0,64,64, 16, 0.2);
    Animation sRock_small(t6, 0,0,64,64, 16, 0.2);
    Animation sBullet(t5, 0,0,32,64, 16, 0.8);
    Animation sPlayer(t1, 40,0,40,40, 1, 0);
    Animation sPlayer_go(t1, 40,40,40,40, 1, 0);
    Animation sExplosion_ship(t7, 0,0,192,192, 64, 0.5);

    std::list<Entity*> entities;
    std::list<Entity*>::iterator it,it1,it2;

    for(int i=0;i<1;i++)
    {
      asteroid *a = new asteroid();
      a->settings(sRock, rand()%W, rand()%H, rand()%360, 25);
      entities.push_back(a);
    }

    space_ship *player = new space_ship();
    player->settings(sPlayer,W/2,H/2,0,20);
    entities.push_back(player);

	bool game_over = false;

    /////main loop/////
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed || (event.type == Event::KeyPressed && event.key.code == Keyboard::Q)){
				app.close();
				break;
			}


            if (event.type == Event::KeyPressed && !game_over )
             if (event.key.code == Keyboard::Space)
              {
                bullet *b = new bullet();
                b->settings(sBullet,player->x,player->y,player->angle,10);
                entities.push_back(b);
              }
        }

	if(!game_over){

	//if (Keyboard::isKeyPressed(Keyboard::Q)) game_over = true;
	if (Keyboard::isKeyPressed(Keyboard::Right)) player->angle+=3;
    if (Keyboard::isKeyPressed(Keyboard::Left))  player->angle-=3;
    if (Keyboard::isKeyPressed(Keyboard::Up)) player->thrust=true;
	else if (Keyboard::isKeyPressed(Keyboard::Down)) player->brake=true;
    else {player->thrust=false;player->brake=false;}



    for(it1=entities.begin();it1!=entities.end();it1++){
		Entity* a = *it1;
     for(it2=entities.begin();it2!=entities.end();it2++)
    {
		Entity* b = *it2;
      if (a->name=="asteroid" && b->name=="bullet")
       if ( isCollide(a,b) )
           {
            a->life=false;
            b->life=false;

			player->score++;

            Entity *e = new Entity();
            e->settings(sExplosion,a->x,a->y);
            e->name="explosion";
            entities.push_back(e);


            for(int i=0;i<2;i++)
            {
             if (a->R==15) continue;
             Entity *e = new asteroid();
             e->settings(sRock_small,a->x,a->y,rand()%360,15);
             entities.push_back(e);
            }

           }

      if (a->name=="player" && b->name=="asteroid")
       if ( isCollide(a,b) )
           {
            b->life=false;
			///////////
			//a->life = false;
			player->lives_left--;

			if(player->lives_left==0){
				game_over = true;

				std::ostringstream curr_type;
				curr_type << player->score;
				player_score_game = curr_type.str();

				if(p_score_file_val < player->score){
					std::ofstream high_score_write;
					high_score_write.open("high_score/high_score.txt");
					player_score_game += "\n";
					high_score_write << player_score_game.c_str();
					high_score_write.close();

				}


			}

			///////////
            Entity *e = new Entity();
            e->settings(sExplosion_ship,a->x,a->y);
            e->name="explosion";
            entities.push_back(e);

            player->settings(sPlayer,W/2,H/2,0,20);
            player->dx=0; player->dy=0;
           }
    }
}
}


    if (player->thrust && !game_over)  player->anim = sPlayer_go;
    else   player->anim = sPlayer;


    for(it=entities.begin();it!=entities.end();it++){
		Entity* e = *it;
		if (e->name=="explosion")
         if (e->anim.isEnd()) e->life=0;
	}


    if (rand()%180==0 && !game_over)
     {
       asteroid *a = new asteroid();
       a->settings(sRock, 0,rand()%H, rand()%360, 25);
       entities.push_back(a);
     }



    for( it=entities.begin();it!=entities.end();)
    {
      Entity *e = *it;

      e->update();
      e->anim.update();

      if (e->life==false) {it=entities.erase(it); delete e;}
      else it++;
    }

	if(!game_over){
		String player_score;
		std::ostringstream curr_type;
		curr_type << player->score;
		player_score = curr_type.str();
		display_score.setString(player_score);
	}
   //////draw//////
   app.draw(background);

   for(it=entities.begin();it!=entities.end();it++){
	   if(!((*it)->name=="player" && game_over))
	   (*it)->draw(app);
   }

   //app.draw(scoreboard);
   //app.draw(lifeboard);

	app.draw(high_score_screen);
	app.draw(high_score_val);

	app.draw(ships_left);

   for(int i = 0; i < player->lives_left; i++){
	   app.draw(life_sprites[i]);
   }
   //if(!game_over)
   app.draw(display_score);

   if(game_over){
	   app.draw(end_text);
	   if(p_score_file_val < player->score){
		app.draw(new_high_score);
	   }
	   //new_high_score.setString(player_score_game);
	   //app.draw(new_high_score);
	   //continue;
   }

   app.display();
    }

	//delete all allocated memory
	for( it=entities.begin();it!=entities.end();it++)
	{
	  Entity *e = *it;
	  it=entities.erase(it);
	  delete e;
	}
    return 0;
}
