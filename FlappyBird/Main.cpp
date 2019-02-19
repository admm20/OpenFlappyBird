#include <iostream>
#include <list>
#include <random>

#include <SFML/Graphics.hpp>


class GameObject : public sf::Sprite
{
public:
	virtual void Update(float) = 0;
	
};

class Player : public GameObject
{
public:
	void Update(float);
	void Jump();
	void ResetPosition();
private:
	float force = 3.0f;
};

class Terrain : public GameObject
{
public:
	void Update(float);
	bool CheckCollision(Player&);
	int type; // 1 - ground, 2 - pipe, 3 - rotated pipe
};

class Application
{
public:
	void Run();

private:
	void LoadContent();
	void SpawnPipes();

	std::list<Terrain> grounds;
	std::list<Terrain> pipes;
	sf::Texture bgTexture;
	sf::Texture birdTexture;
	sf::Texture pipeTexture;
	sf::Texture groundTexture;
	sf::Font font;

	int score = 0;
	float const textureScale = 1.33f;

};

void Player::Update(float deltaTime)
{
	force -= deltaTime * 0.01f;

	sf::Vector2f oldPosition(getPosition());
	setPosition(oldPosition.x, oldPosition.y - force);
}

void Player::ResetPosition()
{
	force = 3.0f;
	setPosition(100, 100);
}

void Player::Jump()
{
	force = 4.0f;
}

void Terrain::Update(float deltaTime)
{
	sf::Vector2f oldPosition(getPosition());
	setPosition(oldPosition.x - (0.1f * deltaTime), oldPosition.y);
}

bool Terrain::CheckCollision(Player &player)
{
	sf::Rect<float> pipeHitbox(this->getPosition(), sf::Vector2f(70.0f, 425.0f));
	sf::Rect<float> playerHitbox(player.getPosition(), sf::Vector2f(45.0f, 32.0f));

	switch (type) 
	{
	case 2:
		if (pipeHitbox.intersects(playerHitbox))
			return true;
		break;
	case 3:
		// image is rotated by 180 degrees, so the hitbox must be moved
		pipeHitbox.left -= 70.0f;
		pipeHitbox.top -= 425.0f;
		if (pipeHitbox.intersects(playerHitbox))
			return true;
		break;
	default:
		break;
	}
	return false;
}

void Application::LoadContent()
{
	bgTexture.loadFromFile("bg.png");
	birdTexture.loadFromFile("bird.png");
	pipeTexture.loadFromFile("pipe.png");
	groundTexture.loadFromFile("ground.png");
	font.loadFromFile("timesbd.ttf");

}

void Application::SpawnPipes()
{
	std::random_device device;
	std::mt19937 generator(device());
	std::uniform_int_distribution<int> distribution(210, 450);

	float yRandomPos = distribution(generator);

	Terrain pipe;
	pipe.setTexture(pipeTexture);
	pipe.setScale(textureScale, textureScale);
	pipe.setPosition(400, yRandomPos);
	pipe.type = 2;
	pipes.push_back(pipe);

	pipe.rotate(180);
	pipe.setPosition(469, yRandomPos - 200);
	pipe.type = 3;
	pipes.push_back(pipe);

	// remove pipes and grounds that are far away from screen
	pipes.remove_if([](Terrain &pipe) {return (pipe.getPosition().x < -500); });
	grounds.remove_if([](Terrain &ground) {return (ground.getPosition().x < -500); });
}

void Application::Run()
{
	sf::RenderWindow gameWindow(sf::VideoMode(380, 676, 32), "Flappy Bird",
		sf::Style::Titlebar | sf::Style::Close);
	gameWindow.setFramerateLimit(120);

	LoadContent();

	sf::Sprite bg;
	bg.setTexture(bgTexture);
	bg.setScale(textureScale, textureScale);
	bg.setPosition(0.0f, 0.0f);

	Player player;
	player.setTexture(birdTexture);
	player.setScale(textureScale, textureScale);
	player.ResetPosition();

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(50);
	text.setFillColor(sf::Color::White);
	text.setPosition(180, 50);

	Terrain ground;
	ground.setTexture(groundTexture);
	ground.setScale(textureScale, textureScale);
	ground.setPosition(0, 550);
	ground.type = 1;
	grounds.push_back(ground);

	sf::Clock clock;
	sf::Clock pipeSpawnClock;
	sf::Clock scoreClock;

	bool holdingSpaceButton = false;
	bool gameLost = false;

	while (gameWindow.isOpen())
	{
		sf::Event ev;
		while (gameWindow.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed) 
			{
				gameWindow.close();
			}
		}

		float elapsedTime = clock.getElapsedTime().asMilliseconds();
		clock.restart();

		// Keyboard input
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) 
		{

			if (!holdingSpaceButton) 
			{
				holdingSpaceButton = true;
				player.Jump();
			}
		}
		else
			holdingSpaceButton = false;

		// Update player position
		player.Update(elapsedTime);

		// Check if player collided with pipes
		for (auto &pipe : pipes)
		{
			if (pipe.CheckCollision(player))
				gameLost = true;
		}

		if (gameLost || player.getPosition().y > 520 || player.getPosition().y < -100)
		{
			gameLost = false;
			player.ResetPosition();
			pipes.clear();
			pipeSpawnClock.restart();
			scoreClock.restart();
			score = 0;
		}

		// Update grounds positions
		for (auto &singleGround : grounds) 
		{
			singleGround.Update(elapsedTime);
		}

		// Create new ground if last one is going out of screen
		auto &lastGround = grounds.back();
		if (lastGround.getPosition().x < -48.0f) 
		{
			ground.setPosition(336, 550);
			grounds.push_back(ground);
		}

		// Update position of pipes
		for (auto &singlePipe : pipes)
		{
			singlePipe.Update(elapsedTime);
		}

		// Spawn new pipes every 3 seconds and give points
		if (pipeSpawnClock.getElapsedTime().asSeconds() > 3.0f) {
			pipeSpawnClock.restart();
			SpawnPipes();

			if (scoreClock.getElapsedTime().asSeconds() > 5.0f)
				score++;
		}

		// Draw every game object
		gameWindow.clear(sf::Color(255, 255, 255, 255));

		gameWindow.draw(bg);
		for (auto &singleTerrain : pipes) {
			gameWindow.draw(singleTerrain);
		}
		for (auto &singleTerrain : grounds) {
			gameWindow.draw(singleTerrain);
		}
		gameWindow.draw(player);
		text.setString(std::to_string(score));
		gameWindow.draw(text);

		gameWindow.display();
	}
}

int main()
{
	Application *app = new Application();
	app->Run();
	delete app;

	return 0;
}