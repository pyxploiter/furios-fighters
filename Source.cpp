#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
using namespace std;

//----------------------------------------- Global Initialization ----------------------------------------------//

const int SCREEN_WIDTH = 1250;
const int SCREEN_HEIGHT = 740;

SDL_Window* window = NULL;
SDL_Renderer* render = NULL;
TTF_Font* font = NULL;

enum GameState { LOADING, SELECTING, START, PAUSE, EXIT, WIN, LOSE };
GameState state = LOADING;
enum SelectFighter { EMPTY, BOXERBOY, DOLLY, FURY, GENDA, SOLDIER, JACKEY };
SelectFighter selected_fighter = EMPTY;

Uint32 callback_loading(Uint32 interval, void* param)
{
	//Print callback message
	printf("Callback: %s\n", (char*)param);

	if (state == SELECTING)
		state = START;
	else if (state == LOADING)
		state = SELECTING;
	return 0;
}

//----------------------------------------- Texture Class Start -----------------------------------------------//

class Texture{
public:
	//Initializes variables
	Texture();

	//Deallocates memory
	~Texture();
	void draw(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
	//Loads image at specified path
	bool load_image(std::string path);
	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Gets image dimensions
	int getWidth();
	int getHeight();
	//The actual hardware texture
	SDL_Texture* mTexture;
	//Image dimensions
	int mWidth;
	int mHeight;
};

Texture::Texture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

Texture::~Texture()
{
	//Deallocate
	free();
}

bool Texture::load_image(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(render, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool Texture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(render, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	//Return success
	return mTexture != NULL;
}

void Texture::draw(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip){
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(render, mTexture, clip, &renderQuad, angle, center, flip);
}

void Texture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void Texture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

int Texture::getWidth()
{
	return mWidth;
}

int Texture::getHeight()
{
	return mHeight;
}

//----------------------------------------- FighterName Class Start -----------------------------------------------//

class Fighter{
private:
	Texture fighter1p;
	Texture fighter2p;
	Texture current_sprite;
	int Pos1X, Pos1Y;
	int Pos2X, Pos2Y;
	int speed;
public:
	Fighter();
	void control(SDL_Event& e);
	void draw_image();
	void playerPosition();
	bool collideScreen_left();
	bool collideScreen_right();
};

Fighter::Fighter(){
	switch (selected_fighter){
	case BOXERBOY:
		fighter1p.load_image("assets/Fighters/Boxerboy.png");
		break;
	case DOLLY:
		fighter1p.load_image("assets/Fighters/Dolly.png");
		break;
	case FURY:
		fighter1p.load_image("assets/Fighters/Fury.png");
		break;
	case GENDA:
		fighter1p.load_image("assets/Fighters/Genda.png");
		break;
	case SOLDIER:
		fighter1p.load_image("assets/Fighters/Soldier.png");
		break;
	case JACKEY:
		fighter1p.load_image("assets/Fighters/Jackey.png");
		break;
	default:
		cout << "Error loading Fighter" << endl;
	}
	fighter2p.load_image("assets/Fighters/Fury.png");
	/*switch (selected_fighter){
	case BOXERBOY:
	fighter2p.load_image("assets/Fighters/Boxerboy.png");
	break;
	case DOLLY:
	fighter2p.load_image("assets/Fighters/Dolly.png");
	break;
	case FURY:
	fighter2p.load_image("assets/Fighters/Fury.png");
	break;
	case GENDA:
	fighter2p.load_image("assets/Fighters/Genda.png");
	break;
	case SOLDIER:
	fighter2p.load_image("assets/Fighters/Soldier.png");
	break;
	case JACKEY:
	fighter2p.load_image("assets/Fighters/Jackey.png");
	break;
	default:
	cout << "Error loading Fighter" << endl;
	}*/
}

bool Fighter::collideScreen_left(){
	if (Pos1X < 1)
		return true;
	return false;
}
bool Fighter::collideScreen_right(){
	if (Pos1X > 1245)
		return true;
	return false;
}

//----------------------------------------- GamePlay Class Start -----------------------------------------------//

class GamePlay{
private:
	Texture loadingScreen;
	Texture playerSelect[6];
	Texture selected;
	Texture stage1;
	Texture logo;
	Texture ready;
	Texture go;
	Texture count_down;
	SDL_Rect select_rect;

public:
	GamePlay(){}
	//Starts up SDL and creates window
	bool init();
	void select_menu(SDL_Event eve);
	//Loads media
	bool loadMedia();
	//Frees media and shuts down SDL
	void close();
	void start();
	bool checkFighter(SDL_Event e, int x1, int x2, int y1, int y2);
	//Uint32 callback(Uint32 interval, void* param);
};

bool GamePlay::init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		window = SDL_CreateWindow("Furious FighterNames", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (render == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(render, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				//Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}
	return success;
}

bool GamePlay::loadMedia()
{
	//Loading success flag
	bool success = true;
	loadingScreen.load_image("assets/Extra/loadingScreen.png");
	playerSelect[0].load_image("assets/SelectMenu/player1.png");
	playerSelect[1].load_image("assets/SelectMenu/player2.png");
	playerSelect[2].load_image("assets/SelectMenu/player3.png");
	playerSelect[3].load_image("assets/SelectMenu/player4.png");
	playerSelect[4].load_image("assets/SelectMenu/player5.png");
	playerSelect[5].load_image("assets/SelectMenu/player6.png");
	selected.load_image("assets/SelectMenu/selected.png");
	stage1.load_image("assets/Stages/stage1.png");
	logo.load_image("assets/Extra/logo.png");
	ready.load_image("assets/Extra/ready.png");
	go.load_image("assets/Extra/go.png");

	font = TTF_OpenFont("assets/font.ttf", 38);
	return success;
}

void GamePlay::select_menu(SDL_Event eve){
	int y1 = 560, y2 = 690;
	static bool select = false;
	if (!select){
		if (checkFighter(eve, 150, 309, y1, y2)){
			SDL_RenderCopy(render, playerSelect[0].mTexture, NULL, NULL);
			select_rect.x = 155; select_rect.y = y1;
			select_rect.w = 116;	select_rect.h = 50;
			if (eve.type == SDL_MOUSEBUTTONDOWN)
				selected_fighter = SOLDIER;
		}
		else if (checkFighter(eve, 310, 475, y1, y2)){
			SDL_RenderCopy(render, playerSelect[1].mTexture, NULL, NULL);
			select_rect.x = 320; select_rect.y = y1;
			select_rect.w = 116;	select_rect.h = 50;
			if (eve.type == SDL_MOUSEBUTTONDOWN)
				selected_fighter = BOXERBOY;
		}
		else if (checkFighter(eve, 476, 630, y1, y2)){
			SDL_RenderCopy(render, playerSelect[2].mTexture, NULL, NULL);
			select_rect.x = 495; select_rect.y = y1;
			select_rect.w = 116;	select_rect.h = 50;
			if (eve.type == SDL_MOUSEBUTTONDOWN)
				selected_fighter = DOLLY;
		}
		else if (checkFighter(eve, 631, 800, y1, y2)){
			SDL_RenderCopy(render, playerSelect[3].mTexture, NULL, NULL);
			select_rect.x = 660; select_rect.y = y1;
			select_rect.w = 116;	select_rect.h = 50;
			if (eve.type == SDL_MOUSEBUTTONDOWN)
				selected_fighter = GENDA;
		}
		else if (checkFighter(eve, 801, 960, y1, y2)){
			SDL_RenderCopy(render, playerSelect[4].mTexture, NULL, NULL);
			select_rect.x = 830; select_rect.y = y1;
			select_rect.w = 116;	select_rect.h = 50;
			if (eve.type == SDL_MOUSEBUTTONDOWN)
				selected_fighter = JACKEY;
		}
		else if (checkFighter(eve, 961, 1110, y1, y2)){
			SDL_RenderCopy(render, playerSelect[5].mTexture, NULL, NULL);
			select_rect.x = 985; select_rect.y = y1;
			select_rect.w = 116;	select_rect.h = 50;
			if (eve.type == SDL_MOUSEBUTTONDOWN)
				selected_fighter = FURY;
		}
		else{
			select_rect.x = 0; select_rect.y = 0;
			select_rect.w = 0;	select_rect.h = 0;
			SDL_RenderCopy(render, playerSelect[0].mTexture, NULL, NULL);
			selected_fighter = EMPTY;
		}
		if (selected_fighter != EMPTY)
			select = true;
	}
	if (select){
		SDL_RenderCopyEx(render, selected.mTexture, NULL, &select_rect, -10, NULL, SDL_FLIP_NONE);
		SDL_TimerID timerID = SDL_AddTimer(1500, callback_loading, "1.5 seconds waited! Selection Menu to start");
	}
}

bool GamePlay::checkFighter(SDL_Event e, int x1, int x2, int y1, int y2)
{
	//Check if mouse is in button
	bool inside = true;
	//If mouse event happened
	if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
	{
		//Get mouse position
		int x, y;
		SDL_GetMouseState(&x, &y);
		/*printf("%d-%d\n", x, y);*/
		//Mouse is left of the button
		if (x < x1)
		{
			inside = false;
		}
		//Mouse is right of the button
		else if (x > x2)
		{
			inside = false;
		}
		//Mouse above the button
		else if (y < y1)
		{
			inside = false;
		}
		//Mouse below the button
		else if (y > y2)
		{
			inside = false;
		}
	}
	return inside;
}

void GamePlay::start()
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Event handler
			SDL_Event eve;
			SDL_TimerID timerID = SDL_AddTimer(2000, callback_loading, "1 seconds waited! Loading Screen to Selection Menu"); //wait for loading screen for 2 seconds
			while (state != EXIT && state != WIN && state != LOSE){
				static int i = 0;
				SDL_PollEvent(&eve);
				if (eve.type == SDL_QUIT || eve.key.keysym.sym == SDLK_ESCAPE){
					break;
				}
				if (state == LOADING){
					SDL_RenderCopy(render, loadingScreen.mTexture, NULL, NULL);
				}
				if (state == SELECTING){
					select_menu(eve);
				}
				if (state == START){
					SDL_RenderCopy(render, stage1.mTexture, NULL, NULL);
					if (i<150){
						logo.draw(100, 100);
						i++;
					}
					if (i<75)
						ready.draw((SCREEN_WIDTH / 2 - ready.mWidth / 2), 600);
					else if (i<150)
						go.draw((SCREEN_WIDTH / 2 - go.mWidth / 2), 600);
					else{
						static int counter = 3660;
						int text = counter / 60;
						count_down.loadFromRenderedText(to_string(text), { 255, 255, 255 });	//SDL_Color white = {255,255,255}
						count_down.draw((SCREEN_WIDTH / 2 - count_down.mWidth / 2), 20);
						if (counter != 0) counter--;
					}
				}
				//Update screen
				SDL_RenderPresent(render);
			}
			SDL_RemoveTimer(timerID);
		}
	}
	//Free resources and close SDL
	close();
}

void GamePlay::close()
{
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_DestroyRenderer(render);
	render = NULL;
	TTF_CloseFont(font);
	font = NULL;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

//----------------------------------------- Main Function Start -----------------------------------------------//

int main(int argc, char* args[])
{
	GamePlay game;
	game.start();
	return 0;
}

