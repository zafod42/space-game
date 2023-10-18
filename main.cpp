#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

class Spaceship {
private:
    float x, y;
    bool invincible;
    sf::Clock invincibilityTimer;

public:
    Spaceship() : x(200), y(250), invincible(false) {}

    void moveLeft() {
        float targetX = x - 10;
        x = lerp(x, targetX, 0.2f);
    }

    void moveRight() {
        float targetX = x + 10;
        x = lerp(x, targetX, 0.2f);
    }

    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    int getX() const {
        return static_cast<int>(x);
    }

    int getY() const {
        return static_cast<int>(y);
    }

    bool isInvincible() const {
        return invincible;
    }

    void setInvincible(bool value) {
        invincible = value;
        if (value) {
            invincibilityTimer.restart();
        }
    }

    bool isInvincibilityExpired() const {
        return invincibilityTimer.getElapsedTime().asSeconds() >= 2.0; // Бессмертие длится 2 секунды
    }
};

class Asteroid {
private:
    float x, y;
    float fallSpeed;

public:
    Asteroid() : fallSpeed(2) {
        reset();
    }

    void incSpeed()
    {
        fallSpeed += 0.1;
    }

    void reset() {
        x = rand() % 400;
        y = -50; // Появляется за верхней границей экрана
    }

    void fall() {
        y += fallSpeed;
        if (y > 300) {
            reset();
        }
    }

    float getX() const {
        return x;
    }

    float getY() const {
        return y;
    }
};

class Bullet {
private:
    float x, y;
    bool active;
    float bulletSpeed;

public:
    Bullet(float speed) : active(false), bulletSpeed(speed) {}

    void shoot(float spaceshipX, float spaceshipY) {
        x = spaceshipX + 20;
        y = spaceshipY;
        active = true;
    }

    void move() {
        if (active) {
            y -= bulletSpeed;
            if (y < 0) {
                active = false;
            }
        }
    }

    float getX() const {
        return x;
    }

    float getY() const {
        return y;
    }

    bool isActive() const {
        return active;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(400, 300), "Space Game");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    sf::Text scoreText, livesText;
    scoreText.setFont(font);
    livesText.setFont(font);
    scoreText.setCharacterSize(20);
    livesText.setCharacterSize(20);
    scoreText.setPosition(10, 10);
    livesText.setPosition(10, 30);

    float asteroidFallSpeed = 2.0f;
    float bulletSpeed = 5.0f;
    std::vector<Asteroid> asteroids(5);
    std::vector<Bullet> bullets;
    int score = 0;
    int lives = 3;
    int destroyedAsteroids = 0;

    sf::Texture spaceshipTexture, asteroidTexture, bulletTexture;

    if (!spaceshipTexture.loadFromFile("../resources/spaceship.png") ||
        !asteroidTexture.loadFromFile("../resources/asteroid.png") ||
        !bulletTexture.loadFromFile("../resources/bullet.png")) {
        std::cerr << "Failed to load textures!" << std::endl;
        return -1;
    }

    sf::Sprite spaceship(spaceshipTexture);
    sf::Sprite asteroid(asteroidTexture);
    sf::Sprite bullet(bulletTexture);

    Spaceship player;
    Bullet bulletObj(bulletSpeed);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::A) {
                    player.moveLeft();
                }
                else if (event.key.code == sf::Keyboard::D) {
                    player.moveRight();
                }
                else if (event.key.code == sf::Keyboard::Space) {
                    if (!bulletObj.isActive()) {
                        bulletObj.shoot(player.getX(), player.getY());
                    }
                }
            }
        }

        // Move the bullet
        bulletObj.move();

        // Move the asteroids
        for (auto& asteroid : asteroids) {
            asteroid.fall();
        }

        // Check for collisions
        if (!player.isInvincible()) {
            for (auto& asteroid : asteroids) {
                if (player.getX() < asteroid.getX() + 50 &&
                    player.getX() + 50 > asteroid.getX() &&
                    player.getY() < asteroid.getY() + 50 &&
                    player.getY() + 50 > asteroid.getY()) {
                    std::cout << "You collided with an asteroid!" << std::endl;
                    lives--;

                    if (lives > 0) {
                        player.setInvincible(true);
                        // Reset player and asteroids
                        player = Spaceship();
                        for (auto& asteroid : asteroids) {
                            asteroid.reset();
                        }
                    }
                    else {
                        std::cout << "Game Over! Score: " << score << std::endl;
                        return 0;
                    }
                }
            }
        }
        else {
            if (player.isInvincibilityExpired()) {
                player.setInvincible(false);
            }
        }

        // Check for bullet collisions
        for (auto& asteroid : asteroids) {
            if (bulletObj.isActive() &&
                bulletObj.getX() < asteroid.getX() + 50 &&
                bulletObj.getX() + 5 > asteroid.getX() &&
                bulletObj.getY() < asteroid.getY() + 50 &&
                bulletObj.getY() + 5 > asteroid.getY()) {
                asteroid.reset();
                bulletObj = Bullet(bulletSpeed); // Deactivate the bullet
                score++;

                destroyedAsteroids++;
                if (destroyedAsteroids >= 10) {
                    destroyedAsteroids = 0;
                    lives++;
                    // Increase difficulty by increasing fall speed of asteroids
                    asteroidFallSpeed += 0.5f;
                    bulletSpeed += 0.2f;
                }
                for (auto& asteroid : asteroids) {
                    asteroid.incSpeed();
                }
            }
        }

        window.clear();

        // Draw the spaceship
        spaceship.setPosition(player.getX(), player.getY());
        window.draw(spaceship);

        // Draw the asteroids
        for (const auto& asteroid : asteroids) {
            sf::Sprite asteroidSprite(asteroidTexture);
            asteroidSprite.setPosition(asteroid.getX(), asteroid.getY());
            window.draw(asteroidSprite);
        }

        // Draw the bullet if active
        if (bulletObj.isActive()) {
            bullet.setPosition(bulletObj.getX(), bulletObj.getY());
            window.draw(bullet);
        }

        // Update and draw the score and lives
        scoreText.setString("Score: " + std::to_string(score));
        livesText.setString("Lives: " + std::to_string(lives));
        window.draw(scoreText);
        window.draw(livesText);

        window.display();
    }

    return 0;
}
